/**
 *  @file logger.c
 */

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
#include "psas_packet.h"
#include "logger.h"
#include "utils_sockets.h"
#include "net_addrs.h"

#define _PASTE(a, b) a##b
#define PASTE(a, b) _PASTE(a, b)
#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)

#define LOGFILE_DIGITS 3
#define LOGFILE_BASE "logfile-"

#define P_LIMIT 1500

static FILE *fp = NULL;
static char log_buffer[1500]; 		// Global so destructor can flush final data
static int log_buffer_size = 0;
static int sd;

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

void logger_init() {
	open_logfile();
	setbuf(fp, NULL);

	// Outgoing socket (WiFi)
	sd = get_send_socket();

	// Initialize sequence number
	sequence = 0;

	// Add sequence number to the first packet
	memcpy(&log_buffer[log_buffer_size], &sequence, sizeof(uint32_t));
	log_buffer_size += sizeof(uint32_t);
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
//	printf("\nDumping packet to disk and wifi.\n\n");
	// for the log file, convert the sequence number to a SEQN message
	message_header header = { .ID="SEQN", .timestamp={0,0,0,0,0,0}, .data_length=htons(4) };
	fwrite(&header, 1, sizeof(message_header), fp);
	fwrite(log_buffer, sizeof(char), log_buffer_size, fp);

	// Send current buffer to WiFi
	sendto_socket(sd, log_buffer, log_buffer_size, WIFI_IP, WIFI_PORT);

	// Reset buffer size
	log_buffer_size = 0;

	// Increment sequence number
	sequence++;

	// Write sequence number to head of next packet
	uint32_t s  = htonl(sequence);
	memcpy(&log_buffer[log_buffer_size], &s, sizeof(uint32_t));
	log_buffer_size += sizeof(uint32_t);

//	printf("Filling packet: ");
}

static void logg(void *data, size_t len)
{
	// Check size of buffer, if big enough, we can send packet
	if (log_buffer_size + len >= P_LIMIT)
		flush_log();

	// Copy data into packet buffer
	memcpy(log_buffer + log_buffer_size, data, len);
	log_buffer_size += len;
//	printf("-");
}

static void log_message(char *msg)
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

void log_receive_arm(char* code){
	// single byte: arm if nonzero
	log_message(code[0] ? "ARM" : "DISARM");
}
void log_receive_rc(RollServoMessage* data){
	data->data_length = htons(data->data_length);
	logg(data, sizeof(RollServoMessage));
}

