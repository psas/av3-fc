/* dumb flight computer
 *
 * Read from a variety of devices:
 * 1. several GFEs:  Power, IMU 1, IMU 2
 * 2. GPS, USB serial device
 * 3. a fifo written to by launch control logged in via ssh to assert rocket ready
 *
 * All data read is dumped in two places
 * 1. network connection to ground (telemetry)
 * 2. log file
 */

/* Possibly easiest is to start from the old fcfifo and ltc-fc-common projects */
/*  fcfifo/net.c */
/*	ltc-fc-common/net*.c */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <netinet/in.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <libusb.h>

#include "libusb-util.h"

#define FOURCC(a,b,c,d) htonl(((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

#define REMOTE_PORT 5005

#define LINK_MTU 1500
#define UDP_HEADER_SIZE 8
#define IPv4_MAX_HEADER_SIZE 60

static FILE *logfile;
static int net_fd;

static struct timespec starttime;

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

static void flush_buffers(void)
{
	send(net_fd, &bucket, pos - (char *) &bucket, 0);
	pos = bucket.buf;
	bucket.seq = htonl(ntohl(bucket.seq) + 1);
	fflush(logfile);
}

static void write_tagged_message(uint32_t fourcc, const char *buf, uint16_t len)
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

static int dummy_open(void)
{
	return -1;
}

static void dummy_read(int fd)
{
}

static const struct fd_source {
	int (*open)(void);
	void (*read)(int fd);
} fd_sources[] = {
	{ dummy_open, dummy_read },
};

static bool is_aps(libusb_device * device){
    struct libusb_device_descriptor descr;
    int retErr = libusb_get_device_descriptor(device, &descr);
    if(retErr){
        print_libusb_error(retErr,"is_aps libusb_get_device_descriptor");
        return false;
    }
    if(descr.idVendor == 0xFFFF && descr.idProduct == 0x0006){
        //todo: more ID methods
        return true;
    }
    return false;
}

static bool is_imu(libusb_device * device){
    struct libusb_device_descriptor descr;
    int retErr = libusb_get_device_descriptor(device, &descr);
    if(retErr){
        print_libusb_error(retErr,"is_imu libusb_get_device_descriptor");
        return false;
    }
    if(descr.idVendor == 0xFFFF && descr.idProduct == 0x0005){
        //todo: more ID methods
        return true;
    }
    return false;
}

int main(int argc, char **argv)
{
	int i, usbErr;
	libusb_context * usb_ctx = NULL;
	libusb_device_handle * aps_handle = NULL;
	libusb_device_handle * imu_handle = NULL;

    usbErr = libusb_init(&usb_ctx);
    if(usbErr){
        print_libusb_error(usbErr, "libusb_init");
        exit(EXIT_FAILURE);
    }
    libusb_set_debug(usb_ctx, 3);

    aps_handle = open_usb_device_handle(usb_ctx, is_aps, iface_nums, 1);
    imu_handle = open_usb_device_handle(usb_ctx, is_imu, iface_nums, 1);


	logfile = fopen("log", "w");
	if (logfile == NULL)
		exit(1);

	// init UDP socket
	struct sockaddr_in remote = { AF_INET };
	remote.sin_port = htons(REMOTE_PORT);
	remote.sin_addr.s_addr = INADDR_BROADCAST;

	net_fd = socket(AF_INET, SOCK_DGRAM, 0);
	int broadcast_flag = 1;
	setsockopt(net_fd, SOL_SOCKET, SO_BROADCAST, &broadcast_flag, sizeof(broadcast_flag));
	connect(net_fd, (struct sockaddr *) &remote, sizeof(struct sockaddr));

	// initialize devices
	const int num_fds = (sizeof(fd_sources) / sizeof(*fd_sources));
	struct pollfd fds[num_fds];

	for (i = 0; i < num_fds; ++i)
		fds[i].fd = fd_sources[i].open();

	clock_gettime(CLOCK_MONOTONIC, &starttime);

	write_tagged_message(FOURCC('L', 'O', 'G', 'S'), "initialized", sizeof("initialized") - 1);
	flush_buffers();

	while (1)
	{
		int n = poll(fds, num_fds, -1);
		if(n < 0 && errno != EINTR)
		{
			fprintf(logfile, "\npoll: %s\n", strerror(errno));
		}
		for(i = 0; n > 0; --n)
		{
			while(!fds[i].revents)
				++i;
			if(fds[i].revents & POLLIN)
				fd_sources[i].read(fds[i].fd);
			if(!(fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)))
				continue;
		}
	}
	return 0;
}
