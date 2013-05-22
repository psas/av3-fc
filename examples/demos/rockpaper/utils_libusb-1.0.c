/***
*
*	utils_libusb-1.0.c
*
*/

#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <libusb-1.0/libusb.h>
#include <unistd.h>

#include "fcfutils.h"

static libusb_context *context = NULL;
static int opencount = 0;

static struct timeval nonblocking = {
		.tv_sec = 0,
		.tv_usec = 0,
};



/**
*
*	libusb callback function to handle events
*
*/
static void libusb_cb(struct pollfd * fd) {
	libusb_handle_events_timeout(context, &nonblocking);
}


static void usb_fd_added_cb(int fd, short events, void * source){
	//printf("Adding this fd after the fact: %d\n", fd);
	
	if(fd){
		fcf_add_fd(fd, events, libusb_cb);
	}
}


static void usb_fd_removed_cb(int fd, void* source){
	fcf_remove_fd (fd);
}

struct libusb_transfer * start_usb_interrupt_transfer(libusb_device_handle * handle, unsigned int ep,
		libusb_transfer_cb_fn cb, void * data, int packet_size, unsigned int timeout) {

	if (packet_size < 0) {
		//"auto-detect" if not specified
		packet_size = libusb_get_max_packet_size (libusb_get_device(handle), ep);
	}

	struct libusb_transfer *transfer = libusb_alloc_transfer(0);

	if(transfer == NULL){
		libusb_free_transfer(transfer);
		return NULL;
	}

	unsigned char *buf  = calloc(packet_size, sizeof(unsigned char));

	if(buf == NULL){
		libusb_free_transfer(transfer);
		return NULL;
	}

	libusb_fill_interrupt_transfer(transfer, handle, ep, buf, packet_size, cb, data, timeout);

	transfer->flags = LIBUSB_TRANSFER_FREE_BUFFER;	// libusb_free_transfer() will free(buf)
	int usb_err = libusb_submit_transfer(transfer);

	if(usb_err != 0){
		libusb_cancel_transfer(transfer);
		libusb_free_transfer(transfer);
		fprintf(stderr, "USB ERROR %d\n", usb_err);
		return NULL;
	}

    return transfer;
}


static libusb_device_handle * get_handle(char *dev_name, libusb_device ** devs, libusb_device * device){
	int iface_num[1] = {0}, rc;
	libusb_device_handle *handle;

	rc = libusb_open(device, &handle);
	if(rc!=0){
		fprintf(stderr, "[%s] libusb: Error code on open: %s.\n", dev_name, libusb_error_name(rc));
		return NULL;
	}

	// double check handle is not NULL
	if (handle == NULL) {
		fprintf(stderr, "[%s] libusb: No handle found.\n", dev_name);
        return NULL;
    }

	rc = libusb_kernel_driver_active(handle, iface_num[0]);
	if(rc < 0){
		fprintf(stderr, "[%s] libusb: Failure finding kernel driver status.\n", dev_name);
		libusb_close(handle);
		return NULL;
	}
	if(rc > 0){ 
		rc = libusb_detach_kernel_driver(handle, iface_num[0]);
		if(rc){
			fprintf(stderr, "[%s] libusb: Could not detach kernel driver.\n", dev_name);
			libusb_close(handle);
			return NULL;
		}
	}

	if(libusb_claim_interface(handle, 0) < 0){ 
		fprintf(stderr, "[%s] libusb: Could not claim interface\n", dev_name);
		libusb_close(handle); 
		return NULL; 
	} 

	return handle;
}


static libusb_device * find_device(libusb_device ** devices, int cnt, int vid, int pid){
	
	struct libusb_device_descriptor desc;
	int i, error;

	// Cycle through list of USB devices and see if one matches
	// the product and vendor IDs of the mouse. 
	for(i=0; i<cnt; i++){
		error = libusb_get_device_descriptor(devices[i], &desc);
		if(error < 0){
			fprintf(stderr, "Could not get device descriptor.\n");
			continue; // If no descriptor, go to next device.
		}

		// Conditional for finding specific device.
		if(desc.idVendor==vid){
			if(desc.idProduct==pid){
				return devices[i];
			}
		}
	}
	return NULL;
}


libusb_context * init_libusb(const char *dev_name){

	if (context == NULL){
		int rc = libusb_init(&context);
		if(rc){
			context = NULL; //reset to NULL, just in case
			fprintf(stderr, "[%s] libusb: init failed.\n", dev_name);
			return NULL;
		}
		//libusb_set_debug(context, 3);
		libusb_set_pollfd_notifiers(context, usb_fd_added_cb, usb_fd_removed_cb, NULL);
		
		const struct libusb_pollfd **fds;
		fds = libusb_get_pollfds(context);
		for(int cnt=0; fds[cnt] != NULL; cnt++){
			usb_fd_added_cb(fds[cnt]->fd, fds[cnt]->events, NULL);
		}
		free(fds);
	}

	return context;
}


libusb_device_handle * open_device (char * dev_name, int vid, int pid) {
	int cnt = 0;
	libusb_device_handle *handle = NULL;
	libusb_device **devs = NULL, *device = NULL;

	if (init_libusb(dev_name) == NULL) {
		return NULL;
	}

	// Get a list of all the devices. Return on error.
 	cnt = libusb_get_device_list(context, &devs);
	if(cnt < 0) {
		fprintf(stderr, "[%s] libusb: Could not get device list.\n", dev_name);
		return NULL;
	}

	// Find device in list of USB device. Return on error.
	device = find_device(devs, cnt, vid, pid);
	if(!device){
		fprintf(stderr, "[%s] libusb: No device with matching vid/pid found.\n", dev_name);
		libusb_free_device_list(devs, 1);
		return NULL;
	}

	handle = get_handle(dev_name, devs, device);
	if(handle == NULL){
		fprintf(stderr, "[%s] Error: Invalid handle returned by get_handle().\n", dev_name);
		libusb_free_device_list(devs, 1);
		return NULL;
	}

	libusb_free_device_list(devs, 1);
	opencount++;
	return handle;
}

void close_device(libusb_device_handle *handle) {
	if (handle != NULL) {
		libusb_close (handle);
		opencount--;
	}
	if (opencount <= 0 && context != NULL) {
		libusb_exit (context);
		context = NULL;
	}
}

void cancel_transfer(struct libusb_transfer *transfer) {
	if (transfer != NULL) {
		libusb_cancel_transfer(transfer);
		libusb_handle_events_timeout(context, &nonblocking);
		libusb_free_transfer (transfer);
	}
}

