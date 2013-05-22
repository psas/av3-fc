
#include <stdlib.h>
#include <stdio.h>
#include <libusb-1.0/libusb.h>
#include <unistd.h>

#include "module_mouse_clark.h"
#include "utils_libusb-1.0.h"


/**	START DATA */
// Microsoft Corp. Optical Mouse
static const int VID = 0x045e;
static const int PID = 0x0053;
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
		sendMessage_mouse_clark("mouse_clark", buf, act_len);

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


void init_mouse_clark() {
	handle = open_device("mouse_clark", VID, PID);

	if (handle != NULL) {
		transfer = start_usb_interrupt_transfer(handle, EPT, data_callback, NULL, -1, 0);
	}
}

void finalize_mouse_clark() {
	cancel_transfer(transfer);
	close_device(handle);
	handle = NULL;
	transfer = NULL;
}
