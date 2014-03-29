#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/timerfd.h>
#include "../elderberry/fcfutils.h"
#include "utilities/utils_sockets.h"
#include "utilities/psas_packet.h"
#include "utilities/net_addrs.h"
#include "logger.h"


/*
 * because of preprocessor shenanigans, macro defined constants need to pass
 * through two layers of macro function to correctly stringify.
 */

#define _PASTE(a, b) a##b
#define PASTE(a, b) _PASTE(a, b)
#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)

#define LOGFILE_DIGITS 3
#define LOGFILE_BASE "logfile-"

#define LINK_MTU 1500
#define UDP_HEADER_SIZE 8
#define IPv4_MAX_HEADER_SIZE 60
#define P_LIMIT LINK_MTU - UDP_HEADER_SIZE - IPv4_MAX_HEADER_SIZE - sizeof(uint32_t)

#define LOG_TIMEOUT_NS 100e6 //100ms

static FILE *fp = NULL;
static char log_buffer[1500]; 		// Global so destructor can flush final data
static unsigned int log_buffer_size = 0;
static int net_fd;

// sequence number, each UDP packet gets a number
uint32_t sequence;


static void open_logfile(void)
{
	/* Compute 10 ** LOGFILE_DIGITS at compile time. */
	const int attempt_count = PASTE(1e, LOGFILE_DIGITS);

	char buf[sizeof LOGFILE_BASE + LOGFILE_DIGITS];

	int i;
	for(i = 0; i < attempt_count; ++i)
	{
		snprintf(buf, sizeof buf, LOGFILE_BASE "%0" STRINGIFY(LOGFILE_DIGITS) "d", i);
		int fd = open(buf, O_WRONLY | O_CREAT | O_EXCL, 0444);
		if(fd == -1)
		{
			if(errno == EEXIST || errno == EISDIR)
				continue;
			fprintf(stderr, "permanent failure creating logfile %s: %s\n", buf, strerror(errno));
			exit(1);
		}

		fp = fdopen(fd, "w");
		return;
	}

	fprintf(stderr, "tried %d filenames but couldn't create any logfile\n", i);
	exit(1);
}

static void open_socket(void)
{
	net_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(net_fd < 0)
	{
		perror("socket(AF_INET, SOCK_DGRAM, 0)");
		exit(1);
	}

	int broadcast_flag = 1;
	/* ignore errors from allowing broadcast addresses; we'll detect the error at connect, below */
	setsockopt(net_fd, SOL_SOCKET, SO_BROADCAST, &broadcast_flag, sizeof(broadcast_flag));

	struct sockaddr_in remote = { AF_INET };
	remote.sin_port = htons(WIFI_PORT);
	if(!inet_aton(WIFI_IP, &remote.sin_addr))
	{
		fprintf(stderr, "inet_aton(\"" WIFI_IP "\") failed\n");
		exit(1);
	}

	if(connect(net_fd, (struct sockaddr *) &remote, sizeof(struct sockaddr)) < 0)
	{
		perror("could not connect to " WIFI_IP ":" STRINGIFY(WIFI_PORT));
		exit(1);
	}
}

static void log_timeout(struct pollfd * pfd);
void logger_init() {
	open_logfile();
	setbuf(fp, NULL);

	// Outgoing socket (WiFi)
	open_socket();

	// Initialize sequence number
	sequence = 0;

	// Add sequence number to the first packet
	memcpy(&log_buffer[log_buffer_size], &sequence, sizeof(uint32_t));
	log_buffer_size += sizeof(uint32_t);


    int tfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    struct itimerspec  newval;
    newval.it_interval.tv_sec = 0;
    newval.it_interval.tv_nsec = LOG_TIMEOUT_NS;
    newval.it_value.tv_sec = 0;
    newval.it_value.tv_nsec = LOG_TIMEOUT_NS;
    timerfd_settime(tfd, 0, &newval, NULL);
    fcf_add_fd(tfd, POLLIN, log_timeout);
}


void logger_final() {
	if(log_buffer_size > 0){
		fwrite(log_buffer, sizeof(char), log_buffer_size, fp);
	}
	fclose(fp);
}

static void flush_log()
{
	// Send current buffer to disk
	// for the log file, convert the sequence number to a SEQN message
	message_header header = { .ID="SEQN", .timestamp={0,0,0,0,0,0}, .data_length=htons(4) };
	fwrite(&header, 1, sizeof(message_header), fp);
	fwrite(log_buffer, sizeof(char), log_buffer_size, fp);
	// Send current buffer to WiFi
	if(write(net_fd, log_buffer, log_buffer_size) != log_buffer_size)
		perror("flush_log: ignoring error from write(net_fd)");

	// Reset buffer size
	log_buffer_size = 0;

	// Increment sequence number
	sequence++;

	// Write sequence number to head of next packet
	uint32_t s  = htonl(sequence);
	memcpy(&log_buffer[log_buffer_size], &s, sizeof(uint32_t));
	log_buffer_size += sizeof(uint32_t);
}

static void logg(const void *data, size_t len)
{
	// Check size of buffer, if big enough, we can send packet
	if (log_buffer_size + len >= P_LIMIT)
		flush_log();

	// Copy data into packet buffer
	memcpy(log_buffer + log_buffer_size, data, len);
	log_buffer_size += len;
//	printf("-");
}

static void log_timeout(struct pollfd * pfd){
    char buf[8];
    read(pfd->fd, buf, 8); //clears timerfd
    if(log_buffer_size > sizeof(uint32_t)){ //sequence number
        flush_log();
    }
}

static void log_message(const char *msg)
{
	int len = strlen(msg);
	if (log_buffer_size + sizeof(message_header) + len > P_LIMIT)
		flush_log();

	message_header header = { .ID = "MESG", .timestamp = {0,0,0,0,0,0}, .data_length=htons(len) };
	logg(&header, sizeof(message_header));
	logg(msg, len);
}

void log_receive_adis(ADISMessage *data) {
	data->data_length = htons(data->data_length);
	logg(data, sizeof(ADISMessage));
}

void log_receive_gps(GPSMessage* data){
	uint16_t len = data->data_length;
	data->data_length = htons(data->data_length);
	// different GPS packets have different lengths
	logg(data, sizeof(message_header) + len);
}

void log_receive_mpu(MPUMessage* data){
	data->data_length = htons(data->data_length);
	logg(data, sizeof(MPUMessage));
}
void log_receive_mpl(MPLMessage* data){
	data->data_length = htons(data->data_length);
	logg(data, sizeof(MPLMessage));
}

void log_receive_arm(const char* code){
	log_message(code);
}

void log_receive_rc(RollServoMessage* data){
	data->data_length = htons(data->data_length);
	logg(data, sizeof(RollServoMessage));
}

void log_receive_rnh(unsigned char *buffer, int unsigned len, unsigned char* timestamp) {

	printf("recv RNH message");

	if (len == sizeof(RNH_Health_Data)) {
		printf("RNH size correct");

		RNHMessage packet = {
			.ID={"RNHH"},
			.timestamp={
				(uint8_t)timestamp[0], (uint8_t)timestamp[1],
				(uint8_t)timestamp[2], (uint8_t)timestamp[3],
				(uint8_t)timestamp[4], (uint8_t)timestamp[5]},
			.data_length=htons(sizeof(RNH_Health_Data))
		};
		// Copy in data from socket
		memcpy(&packet.data, buffer, sizeof(RNH_Health_Data));

		logg(&packet, sizeof(RNHMessage));
	}
}
