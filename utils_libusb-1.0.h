/*
 * utils_libusb-1.0.h
 *
 */

#ifndef UTILS_LIBUSB_H_
#define UTILS_LIBUSB_H_

libusb_context * init_libusb(const char *dev_name);
libusb_device_handle * open_device (char * dev_name, int vid, int pid);
void close_device(libusb_device_handle *handle);
struct libusb_transfer * start_usb_interrupt_transfer(libusb_device_handle * handle, unsigned int ep, libusb_transfer_cb_fn cb, void * data, int packet_size, unsigned int timeout);
void cancel_transfer(struct libusb_transfer *transfer);

#endif /* UTILS_LIBUSB_H_ */
