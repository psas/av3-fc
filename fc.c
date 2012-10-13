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

#include <glib.h>
#include <glib-unix.h>
#include <libusb.h>
#include <stdio.h>
#include <stdlib.h>

#include "logging.h"
#include "gps-gsource.h"
#include "libusb-gsource.h"
#include "aps.h"
#include "theo-imu.h"

static void libusb_mainloop_error_cb(int timeout, int handle_events, GMainLoop * loop){
    if(timeout)
        print_libusb_error(timeout, "libusb timeout");
    if(handle_events)
        print_libusb_error(handle_events, "libusb handle_events");
    printf("quit libusb\n");
    g_main_loop_quit(loop);
}

static gboolean shutdown_gracefully(void *closure)
{
	GMainLoop *main_loop = closure;
	g_main_loop_quit(main_loop);
	return FALSE; /* if this signal happens again, let the process die */
}

int main(int argc, char **argv)
{
	GError *error = NULL;
	GOptionContext *option_context = g_option_context_new("- control the PSAS AV3 flight computer");
	g_option_context_add_group(option_context, options_gps());
	if(!g_option_context_parse(option_context, &argc, &argv, &error))
	{
		printf("option parsing failed: %s\n", error->message);
		exit(1);
	}
	g_option_context_free(option_context);

	GMainLoop * fc_main = g_main_loop_new(NULL, FALSE);
	g_unix_signal_add(SIGINT, shutdown_gracefully, fc_main);
	g_unix_signal_add(SIGTERM, shutdown_gracefully, fc_main);

	libusbSource * usb_source = libusbSource_new();
	if(usb_source == NULL){
		exit(1);
	}
	g_source_set_callback((GSource*) usb_source,
			(GSourceFunc)libusb_mainloop_error_cb, &fc_main, NULL);
	g_source_attach((GSource*) usb_source, NULL);

	init_logging();
	init_aps(usb_source);
	init_theo_imu(usb_source);
	init_gps();

	g_source_unref((GSource*) usb_source);

	g_main_loop_run(fc_main);

	g_main_loop_unref(fc_main);
	g_main_context_unref(g_main_context_default());

	flush_buffers();
	return 0;
}
