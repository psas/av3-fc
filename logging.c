#include "logging.h"

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <netinet/in.h>
#include <sys/stat.h>

#define DEFAULT_LOGDIR "logs"
#define LOGFILE_DIGITS 3
#define REMOTE_PORT 5005

#define LINK_MTU 1500
#define UDP_HEADER_SIZE 8
#define IPv4_MAX_HEADER_SIZE 60

static gboolean loopback;
static gchar *logdir;

static GOptionEntry options[] = {
	{ "local", 'l', 0, G_OPTION_ARG_NONE, &loopback, "Send telemetry to localhost (default: broadcast)", NULL },
	{ "log-dir", 'd', 0, G_OPTION_ARG_FILENAME, &logdir, "Directory to save logfiles to (default: " DEFAULT_LOGDIR ")", "path" },
	{ NULL },
};

GOptionGroup *options_logging(void)
{
	GOptionGroup *option_group = g_option_group_new(
		"logging",
		"Telemetry Logging Options:",
		"Show telemetry logging options",
		NULL, NULL);
	g_option_group_add_entries(option_group, options);
	return option_group;
}

static FILE *logfile;
static int net_fd;
static struct timespec starttime;

static void open_logfile(void)
{
	if(!logdir)
		logdir = g_strdup(DEFAULT_LOGDIR);

	/* For convenience, try to create logdir, but ignore errors.
	 * We'll detect problems with the logdir below. */
	mkdir(logdir, 0777);

	/* Compute 10 ** LOGFILE_DIGITS at compile time. */
	const int attempt_count = G_PASTE(1e, LOGFILE_DIGITS);

	const int bufsize = strlen(logdir) + LOGFILE_DIGITS + 2;
	char *buf = g_malloc(bufsize);

	int i;
	for(i = 0; i < attempt_count; ++i)
	{
		snprintf(buf, bufsize, "%s/%0" G_STRINGIFY(LOGFILE_DIGITS) "d", logdir, i);
		int fd = open(buf, O_WRONLY | O_CREAT | O_EXCL, 0444);
		if(fd == -1)
		{
		       	if(errno == EEXIST || errno == EISDIR)
				continue;
			printf("permanent failure creating logfile %s: %s\n", buf, strerror(errno));
			exit(1);
		}

		logfile = fdopen(fd, "w");
		return;
	}

	printf("tried %d filenames but couldn't create any logfile in %s\n", i, logdir);
	exit(1);
}

static void open_socket(void)
{
	net_fd = socket(AF_INET, SOCK_DGRAM, 0);

	struct sockaddr_in remote = { AF_INET };
	remote.sin_port = htons(REMOTE_PORT);

	if(loopback)
		remote.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	else
	{
		remote.sin_addr.s_addr = htonl(INADDR_BROADCAST);

		int broadcast_flag = 1;
		setsockopt(net_fd, SOL_SOCKET, SO_BROADCAST, &broadcast_flag, sizeof(broadcast_flag));
	}

	connect(net_fd, (struct sockaddr *) &remote, sizeof(struct sockaddr));
}

void init_logging(void)
{
	open_logfile();
	open_socket();

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

void write_tagged_message(uint32_t fourcc, const void *buf, uint16_t len)
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
