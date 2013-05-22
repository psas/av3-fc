/*
 * libusb-gsource.h
 *
 * Functions for integrating libusb with a glib main event loop. Also some
 * useful libusb utils.
 */

#ifndef LIBUSB_GSOURCE_H_
#define LIBUSB_GSOURCE_H_
#include <libusb.h>
#include <glib.h>


typedef void (*libusbSourceErrorCallback)(int, int, gpointer);
typedef gboolean (*is_device)(libusb_device * device);

/* Opaque type for a GSource that embeds libusb in a Glib main-loop. */
typedef struct libusbSource libusbSource;

libusbSource * libusbSource_new(void);
libusb_device_handle * open_usb_device_handle(libusbSource * usb_source,
    is_device is_device, int * iface_num, int num_ifaces);
void print_libusb_error(int libusberrno, const char* str);
void print_libusb_transfer_error(int status, const char* str);
#endif /* LIBUSB_GSOURCE_H_ */
