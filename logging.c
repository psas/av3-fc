#include "logging.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <netinet/in.h>

#define REMOTE_PORT 5005

#define LINK_MTU 1500
#define UDP_HEADER_SIZE 8
#define IPv4_MAX_HEADER_SIZE 60

static FILE *logfile;
static int net_fd;

static struct timespec starttime;

void init_logging(void)
{
	logfile = fopen("log", "w");
	if (logfile == NULL)
		exit(1);

	struct sockaddr_in remote = { AF_INET };
	remote.sin_port = htons(REMOTE_PORT);
	remote.sin_addr.s_addr = INADDR_BROADCAST;

	net_fd = socket(AF_INET, SOCK_DGRAM, 0);
	int broadcast_flag = 1;
	setsockopt(net_fd, SOL_SOCKET, SO_BROADCAST, &broadcast_flag, sizeof(broadcast_flag));
	connect(net_fd, (struct sockaddr *) &remote, sizeof(struct sockaddr));

	clock_gettime(CLOCK_MONOTONIC, &starttime);

	printf_tagged_message(FOURCC('L', 'O', 'G', 'S'), "initialized");
	flush_buffers();
}

static uint64_t get_timestamp(void)
{
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	now.tv_sec -= starttime.tv_sec;
	now.tv_nsec -= starttime.tv_nsec;
	return now.tv_nsec + (uint64_t) now.tv_sec * 1000000000;
}

static struct {
	uint32_t seq;
	char buf[LINK_MTU - UDP_HEADER_SIZE - IPv4_MAX_HEADER_SIZE - sizeof(uint32_t)];
} bucket = { 0 };
static char *pos = bucket.buf;

void flush_buffers(void)
{
	if(pos == bucket.buf)
		return;
	send(net_fd, &bucket, pos - (char *) &bucket, 0);
	pos = bucket.buf;
	bucket.seq = htonl(ntohl(bucket.seq) + 1);
	fflush(logfile);
}

void write_tagged_message(uint32_t fourcc, const char *buf, uint16_t len)
{
	uint64_t timestamp = get_timestamp();

	struct {
		uint32_t fourcc;
		uint16_t length;
		uint16_t timestamp_hi;
		uint32_t timestamp_lo;
	} tag_header = {
		fourcc,
		htons(len),
		htons(timestamp >> 32),
		htonl(timestamp),
	};

	if (pos - (char *) &bucket + sizeof(tag_header) + len > sizeof(bucket))
		flush_buffers();
	memcpy(pos, &tag_header, sizeof(tag_header));
	pos += sizeof(tag_header);
	memcpy(pos, buf, len);
	pos += len;

	fwrite(&tag_header, sizeof(tag_header), 1, logfile);
	fwrite(buf, len, 1, logfile);
}

void printf_tagged_message(uint32_t fourcc, const char *fmt, ...)
{
	char buf[1024];
	int len;
	va_list ap;

	va_start(ap, fmt);
	len = vsnprintf(buf, sizeof buf, fmt, ap);
	va_end(ap);

	write_tagged_message(fourcc, buf, len);
}
