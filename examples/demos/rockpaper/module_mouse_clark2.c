
#include <stdlib.h>
#include <stdio.h>
#include <libusb-1.0/libusb.h>
#include <unistd.h>

#include "module_mouse_clark2.h"
#include "utils_libusb-1.0.h"


/**	START DATA */
// Razer USA, Ltd Mamba
static const int VID = 0x1532;
static const int PID = 0x000a;
static const int EPT = 0x81;

static libusb_device_handle *handle = NULL;
static struct libusb_transfer *transfer = NULL;


/**	START FUNCTIONS */

static void data_callback(struct libusb_transfer *transfer){
	unsigned char *buf = NULL;
    int act_len;
    int retErr;

	switch(transfer->status){
    case LIBUSB_TRANSFER_COMPLETED:

        buf = transfer->buffer;
        act_len = transfer->actual_length;
        retErr = libusb_submit_transfer(transfer);
		if(retErr){
            //print_libusb_transfer_error(transfer->status, "common_cb resub");
        }

		/**
		*	Data handler:
		*	Place call into code generated space here.
		*/
		sendMessage_mouse_clark2("mouse_clark2", buf, act_len);

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


void init_mouse_clark2() {
	handle = open_device("mouse_clark2", VID, PID);

	if (handle != NULL) {
		transfer = start_usb_interrupt_transfer(handle, EPT, data_callback, NULL, -1, 0);
	}
}

void finalize_mouse_clark2() {
	cancel_transfer(transfer);
	close_device(handle);
	handle = NULL;
	transfer = NULL;
}
