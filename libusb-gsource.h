/*
 * libusb-gsource.h
 *
 *  Created on: Jul 14, 2012
 *      Author: theo
 */

#ifndef LIBUSB_GSOURCE_H_
#define LIBUSB_GSOURCE_H_

typedef void (*libusbSourceErrorCallback)(int, int, GMainLoop *);
typedef gboolean (*is_device)(libusb_device * device);


typedef struct libusbSource {
	GSource source;
	GSList * fds;
	int timeout_error;
	int handle_events_error;
	libusb_context * context;
	//put gmainloop in libusbSource?
} libusbSource;

libusbSource * libusbSource_new(libusb_context * context);
libusb_device_handle * open_usb_device_handle(libusb_context * context,
    is_device is_device, int * iface_num, int num_ifaces);
void print_libusb_error(int libusberrno, const char* str);
void print_libusb_transfer_error(int status, const char* str);
#endif /* LIBUSB_GSOURCE_H_ */
