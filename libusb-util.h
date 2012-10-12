/*
 * libusb-gsource.h
 *
 *  Created on: Jul 14, 2012
 *      Author: theo
 */

#ifndef LIBUSB_GSOURCE_H_
#define LIBUSB_GSOURCE_H_

#include "stdbool.h"

typedef void (*libusbSourceErrorCB)(int, int);
typedef bool (*is_device)(libusb_device * device);



typedef struct libusbSource {
	struct pollfd *fds;
	nfds_t nfds;
	int timeout_error;
	int handle_events_error;
	libusb_context * context;
	int (*prepare)(struct libusbSource *usb_src, int *timeout_);
	int (*check)(struct libusbSource *usb_src);
	int (*dispatch)(struct libusbSource *usb_src, libusbSourceErrorCB errCB, void* user_data);
}libusbSource;

libusbSource * libusbSource_new(libusb_context * context);
void libusbSource_free(libusbSource * src);
libusb_device_handle * open_usb_device_handle(libusb_context * context,
		is_device is_device, int * iface_num, int num_ifaces);
void print_libusb_error(int libusberrno, char* str);
void print_libusb_transfer_error(int status, char* str);
#endif /* LIBUSB_GSOURCE_H_ */
