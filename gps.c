/*
 * gps.c
 *
 *  Created on: Jun 22, 2013
 *      Author: theo
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <libusb-1.0/libusb.h>
#include <arpa/inet.h>

#include "utils_sockets.h"
#include "utils_libusb-1.0.h"
#include "psas_packet.h"
#include "gps.h"

// cf 7.1  Crescent Integrators Manual

struct msg {
	char 		magic[4];		// "$BIN"
	uint16_t	type;			//  1,  2, 80, 93-99
	uint16_t	len;			// 52, 16, 40, 56, 96, 128, 300, 28, 68, 304 
	union {
		char raw[304];
		struct msg99 m99;
		// define more of them if we ever need any
	};
};

/**	START DATA */

//TODO: real VID/PID of USB-serial adapter
static const int VID = 0x0000;	//< CHANGE VID
static const int PID = 0x0000;	//< CHANGE PID
static const int EPT = 0x81;	//< CHANGE IF NEEDED (Default)

/**	START FUNCTIONS */

static int handle_msg99(struct msg99 *m99)
{
	// TODO: real timestamp
	GPS_packet p = { .ID="GP99", .timestamp={0,0,0,0,0,0}, .data_length=htons(sizeof(struct msg99))};
	memcpy(&p.data, m99, sizeof(struct msg99));
	sendGPSData(&p);
	return 0;
}

static int handle_packet(struct msg *m)
{
	switch (m->type)
	{
	case 99:
		if (m->len == 304)
			return handle_msg99(&m->m99);
		fprintf(stderr, "bad length %d != 304\n", m->len);
		return -3;
	default:
		fprintf(stderr, "unhandled packet type: %d length %d\n", m->type, m->len);
		return -4;
	}
}

static unsigned char buffer[4096], *end = buffer;

static void put_data(unsigned char *buf, int datalen)
{
	memcpy(end, buf, datalen);
	end += datalen;
}

static uint16_t sum(uint8_t *packet, int len)
{
	uint16_t s = 0;
	while (len--)
		s += *packet++;
	return s;
}

// return 0 if could parse a good packet out of the data within [buffer-end)
static int get_packet(struct msg *m)
{
	unsigned char *p = buffer;
	uint16_t checksum;

	while ((p = memchr(p, '$', end - p)) != NULL)
	{
		// sync stream to "$BIN"
		if (memcmp(p, "$BIN", 4) != 0) {
			p++;
			continue;
		}
		memcpy(m, p, 8);
		if (m->type > 99 || m->len > 304) {
			fprintf(stderr, "bad header\n");
			p += 4;
			continue;
		}
		if (8 + m->len + 4 < end - p) {
			// incomplete, need more data
			return 0;
		}
		memcpy(&checksum, p + 8 + m->len, 2);
		if (sum(p + 8, m->len) == checksum) {
			// good packet: advance and return success
			memcpy(m->raw, p + 8, m->len);

			p += 8 + m->len + 4;			// header + data + checksum + \r\n
			memmove(buffer, p, end - p);
			end = buffer + (end - p);
			return 1;
		}
		fprintf(stderr, "bad checksum %u (expected %u)\n", sum(p + 8, m->len), checksum);
		p += 4;
	}
	end = buffer;
	return 0;
}

static void data_callback(struct libusb_transfer *transfer){
	unsigned char *buf = NULL;
    int act_len;
    int retErr;
	struct msg m;

	switch(transfer->status){
    
		case LIBUSB_TRANSFER_COMPLETED:

			buf = transfer->buffer;
			act_len = transfer->actual_length;
			retErr = libusb_submit_transfer(transfer);
			if(retErr){
				//print_libusb_transfer_error(transfer->status, "common_cb resub");
			}

			put_data(buf, act_len);
			while (get_packet(&m))
				handle_packet(&m);

			break;
		
		case LIBUSB_TRANSFER_CANCELLED:
			printf("transfer cancelled\n");
			break;
		
		default:
			printf("data_callback() error\n");
			break;
    }
}

struct libusb_transfer * transfer;
libusb_device_handle * handle;
void gps_init() {
	//TODO: correct device name, VID, PID
	libusb_context *context = init_libusb ("###DEVTAG###");
	if (context == NULL) {
		return;
	}
	libusb_set_debug(context, 3);
	handle = open_device("###DEVTAG###", VID, PID);
	if (handle != NULL) {
		transfer = start_usb_interrupt_transfer(handle, EPT, data_callback, NULL, -1, 0);
	}
}

void gps_final() {
	cancel_transfer(transfer);
	close_device(handle);
	handle = NULL;
	transfer = NULL;
}
