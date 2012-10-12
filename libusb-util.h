/*
 * libusb-util.h
 *
 * Handy utilities for libusb
 */

#ifndef LIBUSB_GSOURCE_H_
#define LIBUSB_GSOURCE_H_

#include "stdbool.h"

typedef bool (*is_device)(libusb_device * device);

libusb_device * find_usb_device(libusb_context * context, is_device is_device);
libusb_device_handle * open_device_interface(libusb_device * dev, int * iface_num, int num_ifaces);
libusb_device_handle * open_usb_device_handle(libusb_context * context,
		is_device is_device, int * iface_num, int num_ifaces);
void print_libusb_error(int libusberrno, const char* str);
void print_libusb_transfer_error(int status, const char* str);
#endif /* LIBUSB_GSOURCE_H_ */
