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
#include "utilities/utils_time.h"
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

static char log_buffer[P_LIMIT];  // Global so destructor can flush final data
static unsigned int log_buffer_size = 0;
static char disk_log_buffer[65536];
static unsigned int disk_log_buffer_size = 0;
static int disk_fd;
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

		disk_fd = fd;
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

	if(connect(net_fd, WIFI_ADDR, sizeof(struct sockaddr_in)) < 0)
	{
		perror("could not connect to WIFI");
		exit(1);
	}
}

static void log_timeout(struct pollfd * pfd);
void logger_init() {
	open_logfile();

	// Outgoing socket (WiFi)
	open_socket();

	// Initialize sequence number
	sequence = 0;
	log_buffer_size = 0;
	disk_log_buffer_size = 0;


    int tfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    struct itimerspec  newval;
    newval.it_interval.tv_sec = 0;
    newval.it_interval.tv_nsec = LOG_TIMEOUT_NS;
    newval.it_value.tv_sec = 0;
    newval.it_value.tv_nsec = LOG_TIMEOUT_NS;
    timerfd_settime(tfd, 0, &newval, NULL);
    fcf_add_fd(tfd, POLLIN, log_timeout);
}


static void flush_log()
{
	if (!disk_log_buffer_size)
		return;

	// Send current buffer to disk
	// for the log file, convert the sequence number to a SEQN message
	message_header header = {
		.ID={"SEQN"},
		.data_length=htons(sizeof(SequenceNoData))
	};
	get_psas_time(header.timestamp);

	const struct iovec disk_iov[] = {
		{ .iov_base = &header, .iov_len = sizeof header },
		{ .iov_base = &sequence, .iov_len = sizeof sequence },
		{ .iov_base = disk_log_buffer, .iov_len = disk_log_buffer_size },
	};
	writev(disk_fd, disk_iov, sizeof disk_iov / sizeof *disk_iov);

	const struct iovec net_iov[] = {
		{ .iov_base = &sequence, .iov_len = sizeof sequence },
		{ .iov_base = log_buffer, .iov_len = log_buffer_size },
	};
	writev(net_fd, net_iov, sizeof net_iov / sizeof *net_iov);

	// Reset buffer size
	log_buffer_size = 0;
	disk_log_buffer_size = 0;

	// Increment sequence number
	sequence++;
}

void logger_final() {
	flush_log();
	close(disk_fd);
	close(net_fd);
}

static void ensure_disk_log_space(unsigned int len) {
	if (disk_log_buffer_size + len > sizeof disk_log_buffer)
		flush_log();
}

static void ensure_net_log_space(unsigned int len) {
	ensure_disk_log_space(len);
	if (log_buffer_size + len > sizeof log_buffer)
		flush_log();
}

void log_write_disk_only(const char ID[4], const uint8_t timestamp[6], uint16_t data_length, const void *data)
{
	unsigned int len = sizeof(message_header) + data_length;
	ensure_disk_log_space(len);

	message_header *header = (message_header *) (disk_log_buffer + disk_log_buffer_size);
	memcpy(header->ID, ID, sizeof header->ID);
	memcpy(header->timestamp, timestamp, sizeof header->timestamp);
	header->data_length = htons(data_length);

	memcpy(disk_log_buffer + disk_log_buffer_size + sizeof(message_header), data, data_length);

	disk_log_buffer_size += len;
}

void log_write(const char ID[4], const uint8_t timestamp[6], uint16_t data_length, const void *data)
{
	unsigned int len = sizeof(message_header) + data_length;
	ensure_net_log_space(len);

	unsigned int disk_start = disk_log_buffer_size;
	log_write_disk_only(ID, timestamp, data_length, data);

	memcpy(log_buffer + log_buffer_size, disk_log_buffer + disk_start, len);
	log_buffer_size += len;
}

static void log_timeout(struct pollfd * pfd){
	char buf[8];
	if(read(pfd->fd, buf, 8)<0){ //clears timerfd
		perror("log_timeout: read() failed");
	}
	flush_log();
}

void log_receive_state(VSTEMessage *data) {
	log_write(data->ID, data->timestamp, data->data_length, &data->data);
}

void log_receive_arm(const char* code){
	uint8_t timestamp[6];
	get_psas_time(timestamp);
	log_write("MESG", timestamp, strlen(code), code);
}

void log_receive_rc(ROLLMessage* data) {
	union {
		uint64_t uint;
		double doub;
	} convert;

	convert.doub = data->data.angle;
	convert.uint = __builtin_bswap64(convert.uint);
	data->data.angle = convert.doub;
	log_write(data->ID, data->timestamp, data->data_length, &data->data);
}

void log_receive_rnh_version(uint8_t * message, unsigned int length){
	uint8_t timestamp[6];
	get_psas_time(timestamp);
	log_write("VERS", timestamp, length, message);
}
