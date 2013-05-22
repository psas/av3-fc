
/******************************************************************************
*
*	LIBUSB USER MODULE TEMPLATE
*
*	This template was created to help speed up the process of building user
*	modules that use libusb. To start using this file, follow these steps:
*
*		STEP 1.	Change VendorID (VID) and ProductID (PID) values, below to
*				match those of your device.
*
*		STEP 2.	Search and Replace ###DEVTAG### with the token you've assigned 
*				to your device (e.g. mouse1) in both this file and the 
*				accompanying header file. This step is to ensure that
*				there are not namespace conflicts.
*
*		STEP 3.	Make sure the callback in the data_callback() function is 
*				setup in fcfmain.c and fcfmain.h. Also, make sure the
*				arguments to this callback are sufficient to the needs of
*				the device.
*
*		STEP 3.	Rename both this file and the accompanying header file to
*				a unique filename in the root framework directory that includes
*				the token. For the sake of convention, we recommend 
*				module_{token}.c/.h.
*
*		STEP 4. Create the MIML file for the device by using the header2Miml.py
*				script. Example: ./header2Miml.py module_(token)
*
**/

#include <sys/time.h>
#include <sys/poll.h>
#include <stdlib.h>
#include <stdio.h>
#include <libusb-1.0/libusb.h>
#include <unistd.h>

#include "utils_libusb-1.0.h"

/**	START DATA */

static const int VID = 0x0000;	//< CHANGE VID
static const int PID = 0x0000;	//< CHANGE PID
static const int EPT = 0x81;	//< CHANGE IF NEEDED (Default)

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
			sendMessage_###DEVTAG###("###DEV_TAG###", buf, act_len);

			break;
		
		case LIBUSB_TRANSFER_CANCELLED:
			printf("transfer cancelled\n");
			break;
		
		default:
			printf("data_callback() error\n");
			break;
    }
}


void init_###DEVTAG###() {
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

void finalize_###DEVTAG###() {
	cancel_transfer(transfer);
	close_device(handle);
	handle = NULL;
	transfer = NULL;
}
