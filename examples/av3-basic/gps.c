/*
 * gps.c (based on gps-gsource.c)
 */
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>

#include "miml.h"
#include "gps.h"
#include "fcfutils.h"

static const char *device = NULL;

static unsigned char buf[4096], *cur = buf;


void set_gps_devicepath (const char *dev) {
	device = dev;
}


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
		//JM write_tagged_message(FOURCC('G', 'P', 'S', pos[4]), pos + 8, data_length + 2);
		FCF_Log (FOURCC('G', 'P', 'S', pos[4]), pos + 8, data_length + 2);
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


//returns true/false
static int read_gps_cb (struct pollfd *pfd) {
	if (pfd->revents != POLLIN) {
		return 0;
	}

	ssize_t nread = read (pfd->fd, cur, sizeof(buf)-(cur-buf));

	if (nread <= 0) {
		//error or nothing was read
		return 0;
	}

//	if (nread == -1) {
//		//read returned with error
//		if (errno == EAGAIN) {
//			//read would block
//			return 0;
//		} else {
//			//other error
//			return 0;
//		}
//	}

	cur += nread;
	find_frames();
	return 1;
}


void init_gps(libusbSource * src)	//TODO FCF struct must not be part of libusbSource
{
	if(!device)
		device = "/dev/usbserial";

	int fd = open (device, O_RDONLY | O_NONBLOCK);
	if (fd == -1) {
		fprintf (stderr, "Can't connect to GPS on %s: %s\n", device, strerror(errno));
		return;
	}

	fcf_addfd (fd, POLLIN, read_gps_cb);
}
