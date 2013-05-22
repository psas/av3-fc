/*
 * module_mouse.c
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <libusb-1.0/libusb.h>

#include "module_mouse.h"
#include "utils_libusb-1.0.h"

/**	START DATA */
// SET TO VID/PID of mouse. EPT = Endpoint
static const int VID = 0x0000;
static const int PID = 0x0000;
static const int EPT = 0x81;

static libusb_device_handle *handle = NULL;
static struct libusb_transfer *transfer = NULL;

static void data_callback(struct libusb_transfer *transfer){
	unsigned char *buf = NULL;
	int act_len;
	int retErr;

	switch(transfer->status){
	case LIBUSB_TRANSFER_COMPLETED:

		buf = transfer->buffer;
		act_len = transfer->actual_length;

		retErr = libusb_submit_transfer(transfer);
		//printf("Data from mouse: %02x \n", (char)buf[0]);

		if(retErr){
			//print_libusb_transfer_error(transfer->status, "common_cb resub");
		}

		// Call to CGS mouse handler.
		sendMessage_mouse("mouse", buf, act_len);

		break;
	case LIBUSB_TRANSFER_CANCELLED:
		//do nothing.
		break;
	default:
		//print_libusb_transfer_error(transfer->status, "common_cb");
		printf("data_callback() error\n");
		break;
	}
}


void init_mouse() {

	libusb_context *context = init_libusb ("mouse");
	if (context == NULL) {
		return;
	}
	libusb_set_debug(context, 3);
	handle = open_device("mouse", VID, PID);
	if (handle != NULL) {
		transfer = start_usb_interrupt_transfer(handle, EPT, data_callback, NULL, -1, 0);
	}
}

void finalize_mouse() {
	cancel_transfer(transfer);
	close_device(handle);
	handle = NULL;
	transfer = NULL;
}
