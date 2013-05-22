#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "logging.h"
#include "gps-gsource.h"

static gchar *device;

static GOptionEntry options[] = {
	{ "gps-device", 'g', 0, G_OPTION_ARG_FILENAME, &device, "Device path", NULL },
	{ NULL },
};

GOptionGroup *options_gps(void)
{
	GOptionGroup *option_group = g_option_group_new(
		"gps",
		"GPS Options:",
		"Show GPS options",
		NULL, NULL);
	g_option_group_add_entries(option_group, options);
	return option_group;
}

static unsigned char buf[4096], *cur = buf;

static void find_frames(void)
{
	unsigned char *pos = buf;
	while((pos = memchr(pos, '$', cur - pos)))
	{
		/* Do we have enough for a complete header? */
		if(cur - pos < 8)
			break;

		/* Check the rest of the synchronization string, and
		 * also reject Block IDs greater than 255. */
		if(memcmp(pos + 1, "BIN", 3) != 0 || pos[5] != 0)
		{
			/* Look for the next possible frame. */
			++pos;
			continue;
		}

		/* Have we finished reading the putative packet? */
		int data_length = pos[6] | (pos[7] << 8);
		if(cur - pos < data_length + 12)
			break;

		/* Check the frame epilogue. Ignore the checksum;
		 * we want to see this frame even if mangled. */
		if(memcmp(pos + data_length + 10, "\r\n", 2) != 0)
		{
			/* "$BIN" can't match '$' again. */
			pos += 4;
			continue;
		}

		/* Looks valid. Strip the framing and consume it. */
		write_tagged_message(FOURCC('G', 'P', 'S', pos[4]), pos + 8, data_length + 2);
		pos += data_length + 12;
	}

	/* If we can't find another '$', discard the rest of the buffer.
	 * Otherwise, only discard up to the next '$', and preserve the
	 * rest for the next round. */
	if(pos == NULL)
		cur = buf;
	else
	{
		memmove(buf, pos, cur - pos);
		cur -= pos - buf;
	}
}

static gboolean read_gps_cb(GIOChannel *gps, GIOCondition cond, gpointer data)
{
        if (cond != G_IO_IN)
                return FALSE;

	gsize nread = 0;
	GIOStatus status = g_io_channel_read_chars(gps, (gchar *) cur, sizeof(buf)-(cur-buf), &nread, NULL);
	if (status == G_IO_STATUS_EOF)
		return FALSE;

	cur += nread;
	find_frames();
	return TRUE;
}

void init_gps(void)
{
	if(!device)
		device = g_strdup("/dev/usbserial");

	GIOChannel *gps_source = g_io_channel_new_file(device, "r", NULL);
	if (gps_source == NULL)
	{
		printf("Can't connect to GPS\n");
		return;
	}
	g_io_channel_set_encoding(gps_source, NULL, NULL); /* use binary mode */
	g_io_channel_set_flags(gps_source, G_IO_FLAG_NONBLOCK, NULL);
	g_io_add_watch(gps_source, G_IO_IN, read_gps_cb, NULL);
	g_io_channel_unref(gps_source);
}

