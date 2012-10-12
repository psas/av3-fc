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
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <netinet/in.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <libusb.h>
#include <glib.h>

#include "libusb-gsource.h"
#include "theo-imu.h"

#define FOURCC(a,b,c,d) htonl(((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

#define REMOTE_PORT 5005

#define LINK_MTU 1500
#define UDP_HEADER_SIZE 8
#define IPv4_MAX_HEADER_SIZE 60

#define IOUT_PIN 23
#define ACOK_PIN 11
#define FC_SPS_PIN 5
#define ATV_SPS_PIN 6
#define RC_POWER_PIN 7
#define ROCKET_READY_PIN 8
#define WIFI_POWER_PIN 9
#define RC_TETHER 15



static FILE *logfile;
static int net_fd;
libusb_device_handle * aps_handle = NULL;

GMainLoop * fc_main = NULL;

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

static gboolean is_aps(libusb_device * device){
    struct libusb_device_descriptor descr;
    int retErr = libusb_get_device_descriptor(device, &descr);
    if(retErr){
        print_libusb_error(retErr,"is_aps libusb_get_device_descriptor");
        return false;
    }
    if(descr.idVendor == 0xFFFF && descr.idProduct == 0x0006){
        //todo: more ID methods
        return TRUE;
    }
    return FALSE;
}

static void libusb_mainloop_error_cb(int timeout, int handle_events, GMainLoop * loop){
    if(timeout)
        print_libusb_error(timeout, "libusb timeout");
    if(handle_events)
        print_libusb_error(handle_events, "libusb handle_events");
    printf("quit libusb\n");
    g_main_loop_quit(loop);
}

static void set_port(int port, uint32_t val){
    int setport = 0x80;
    unsigned char data[64];
    int usb_err;
    data[0] = (val & 0xFF<<0)>>0;
    data[1] = (val & 0xFF<<8)>>8;
    data[2] = (val & 0xFF<<16)>>16;
    data[3] = (val & 0xFF<<24)>>24;
    usb_err = libusb_control_transfer(aps_handle,
            LIBUSB_RECIPIENT_OTHER | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT,
            setport | port, 0, 0, data, 4, 2000);
    if(usb_err < 0){
        print_libusb_error(usb_err, "set_port");
    }
    if(usb_err != 4){
        printf("set_port: Didn't send correct number of bytes");
    }
}


#if 0
static void clear_port(int port, uint32_t val){
    int clearport = 0x40;
    unsigned char data[64];
    int usb_err;
    data[0] = (val & 0xFF<<0)>>0;
    data[1] = (val & 0xFF<<8)>>8;
    data[2] = (val & 0xFF<<16)>>16;
    data[3] = (val & 0xFF<<24)>>24;
    usb_err = libusb_control_transfer(aps_handle,
            LIBUSB_RECIPIENT_OTHER | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT,
            clearport | port, 0, 0, data, 4, 2000);
    if(usb_err < 0){
        print_libusb_error(usb_err, "set_port");
    }
    if(usb_err != 4){
        printf("set_port: Didn't send correct number of bytes");
    }
}
#endif


int main(int argc, char **argv)
{
	int usbErr;
	int iface_nums[1] = {0};
	libusbSource * usb_source = NULL;

    usbErr = libusb_init(NULL);
    if(usbErr){
        print_libusb_error(usbErr, "libusb_init");
        exit(EXIT_FAILURE);
    }
    libusb_set_debug(NULL, 3);

    fc_main = g_main_loop_new(NULL, FALSE);


    usb_source = libusbSource_new(NULL);
    if(usb_source == NULL){
        exit(1);
    }
    g_source_set_callback((GSource*) usb_source,
            (GSourceFunc)libusb_mainloop_error_cb, &fc_main, NULL);
    g_source_attach((GSource*) usb_source, NULL);
    aps_handle = open_usb_device_handle(NULL, is_aps, iface_nums, 1);
    if(aps_handle == NULL){
        exit(1);
    }


    set_port(0, (1<<ATV_SPS_PIN) | (1<<RC_POWER_PIN) | (1<<WIFI_POWER_PIN));


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

	clock_gettime(CLOCK_MONOTONIC, &starttime);

	write_tagged_message(FOURCC('L', 'O', 'G', 'S'), "initialized", sizeof("initialized") - 1);
	flush_buffers();
	init_theo_imu();
	g_main_loop_run(fc_main);

	g_source_destroy((GSource*) usb_source);
	g_main_loop_unref(fc_main);

	;
	libusb_exit(NULL);
	return 0;
}
