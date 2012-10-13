#include <stdio.h>
#include <string.h>
#include <glib.h>
#include "logging.h"
#include "crescent.h"
#include "gps-gsource.h"

#define GPS_CODE FOURCC('G','P','S','U')

static gchar *has_packet(gchar *buf, size_t size, size_t *len)
{
	/* find "$BIN" in buf+size */
	/* parse out packet length */
	/* check whether we have that much data, and if it ends with \r\n */
	/* if so, return found $BIN and length of data */
	/* TODO: checksum? return checksum out-of-band? */
	/* return NULL if packet is incomplete */
	return NULL;
}

static gchar buf[4096], *cur = buf;

static gboolean read_gps_cb(GIOChannel *gps, GIOCondition cond, gpointer data)
{
        if (cond != G_IO_IN)
                return FALSE;

	gsize nread = 0;
        g_io_channel_read_chars(gps, cur, sizeof(buf)-(cur-buf), &nread, NULL);
	cur += nread;
	
	size_t len = 0;
	gchar *packet;
	while ((packet = has_packet(buf, cur-buf, &len)))
	{
		write_tagged_message(GPS_CODE, packet, len-2);
		memcpy(buf, packet+len, cur-packet+len);
		cur = buf + (cur-packet+len);  
	}

        return TRUE;
}

void init_gps(void)
{
	GIOChannel *gps_source = g_io_channel_new_file("/dev/usbserial", "r", NULL);
	if (gps_source == NULL)
	{
		printf("Can't connect to GPS\n");
	}
	else
	{
		g_io_channel_set_flags(gps_source, G_IO_FLAG_NONBLOCK, NULL);
		g_io_add_watch(gps_source, G_IO_IN, read_gps_cb, NULL);
	}
}

