#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <glib.h>
#include "logging.h"
#include "crescent.h"
#include "gps-gsource.h"

#define GPS_CODE FOURCC('G','P','S','U')

static void *memstr(const void *m, const char *s, size_t len)
{
	size_t l = strlen(s);
	void *p;
	while ((p = memchr(m, *s, len)) != NULL)
	{
		len -= p-m + 1;
		if (len <= l)
			return NULL;
		if (memcmp(p, s, l) == 0)
			return p;
		m = p + 1;
	}
	return p;
}

static gchar *has_packet(gchar *buf, size_t size, uint16_t *len)
{
	gchar *p = memstr(buf, "$BIN", size);
	if (p == NULL)
		return NULL;

	uint16_t packet_len;
	memcpy(&packet_len, p+6, 2);
	/* check whether we have that much data, and if it ends with \r\n */
	if (size < packet_len + 12U)
		return NULL;
	if (memcmp(p+packet_len+2, "\r\n", 2) != 0)
		return NULL;		//!!! what to do?
	/* TODO: checksum? return checksum out-of-band? */
	*len = packet_len + 12;
	return p;
}

static gchar buf[4096], *cur = buf;

static gboolean read_gps_cb(GIOChannel *gps, GIOCondition cond, gpointer data)
{
        if (cond != G_IO_IN)
                return FALSE;

	gsize nread = 0;
        g_io_channel_read_chars(gps, cur, sizeof(buf)-(cur-buf), &nread, NULL);
	cur += nread;
	
	uint16_t len = 0;
	gchar *packet;
	while ((packet = has_packet(buf, cur-buf, &len)))
	{
		write_tagged_message(GPS_CODE, packet, len);
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

