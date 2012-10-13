/* dumb flight computer
 *
 * Read from a variety of devices:
 * 1. several GFEs:  Power, IMU 1, IMU 2
 * 2. GPS, USB serial device
 * 3. a fifo written to by launch control logged in via ssh to assert rocket ready
 *
 * All data read is dumped in two places
 * 1. network connection to ground (telemetry)
 * 2. log file
 */

/* Possibly easiest is to start from the old fcfifo and ltc-fc-common projects */
/*  fcfifo/net.c */
/*	ltc-fc-common/net*.c */

#include <stdio.h>
#include <stdlib.h>
#include <libusb.h>
#include <glib.h>

#include "logging.h"
#include "gps-gsource.h"
#include "libusb-gsource.h"
#include "aps.h"
#include "theo-imu.h"

GMainLoop * fc_main = NULL;

static void libusb_mainloop_error_cb(int timeout, int handle_events, GMainLoop * loop){
    if(timeout)
        print_libusb_error(timeout, "libusb timeout");
    if(handle_events)
        print_libusb_error(handle_events, "libusb handle_events");
    printf("quit libusb\n");
    g_main_loop_quit(loop);
}

int main(int argc, char **argv)
{
	int usbErr;
	libusbSource * usb_source = NULL;

    usbErr = libusb_init(NULL);
    if(usbErr){
        print_libusb_error(usbErr, "libusb_init");
        exit(EXIT_FAILURE);
    }
    libusb_set_debug(NULL, 3);

    fc_main = g_main_loop_new(NULL, FALSE);


    usb_source = libusbSource_new(NULL);
    if(usb_source == NULL){
        exit(1);
    }
    g_source_set_callback((GSource*) usb_source,
            (GSourceFunc)libusb_mainloop_error_cb, &fc_main, NULL);
    g_source_attach((GSource*) usb_source, NULL);

	init_logging();
	init_aps();
	init_theo_imu();
	init_gps();

	g_main_loop_run(fc_main);

	g_source_destroy((GSource*) usb_source);
	g_main_loop_unref(fc_main);

	;
	libusb_exit(NULL);
	return 0;
}
