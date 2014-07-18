#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "gps.h"


// cf 7.1  Crescent Integrators Manual
struct msg {
	uint8_t		magic[4];		// "$BIN"
	uint16_t	type;			//  1,  2, 80, 93-99
	uint16_t	len;			// 52, 16, 40, 56, 96, 128, 300, 28, 68, 304
	uint8_t		raw[304];
};


static int handle_msg(unsigned char type, const void *data, uint16_t len, unsigned char* timestamp)
{
	GPSMessage p = {
		.ID = "GPS",
		.data_length = len,
	};
	p.ID[3] = type;
	memcpy(&p.timestamp, timestamp, sizeof(p.timestamp));
	memcpy(&p.raw, data, len);
	gps_data_out(&p);
	return 0;
}

static unsigned char buffer[4096], *end = buffer;

static uint16_t sum(uint8_t *packet, int len)
{
	uint16_t s = 0;
	while (len--)
		s += *packet++;
	return s;
}

// return 1 if could parse a good packet out of the data within [buffer-end)
static int get_packet(struct msg *m)
{
	unsigned char *p = buffer;
	uint16_t checksum;

	while ((p = memchr(p, '$', end - p)) != NULL)
	{
		// sync stream to "$BIN"
		if (end - p < 8)
			return 0;	// incomplete, need more data
		if (memcmp(p, "$BIN", 4) != 0) {
			p++;
			continue;
		}
		memcpy(m, p, 8);
		if (m->type > 99 || m->len > 304) {
			fprintf(stderr, "GPS bad header: type %u len %u\n", m->type, m->len);
			p += 4;
			continue;
		}
		if (end - p < 8 + m->len + 4)
			return 0;	// incomplete, need more data
		memcpy(&checksum, p + 8 + m->len, 2);
		if (sum(p + 8, m->len) == checksum) {
			// good packet: advance and return success
			memcpy(m->raw, p + 8, m->len);

			p += 8 + m->len + 4;			// header + data + checksum + \r\n
			memmove(buffer, p, end - p);
			end = buffer + (end - p);
			return 1;
		}
		fprintf(stderr, "GPS(%u:%u) bad checksum %u (expected %u)\n",
			m->type, m->len, sum(p + 8, m->len), checksum);
		p += 4;
	}
	end = buffer;
	return 0;
}

void cots_raw_in(unsigned char* packet, unsigned int len, unsigned char* timestamp)
{
	struct msg m;

	memcpy(end, packet, len);
	end += len;

	while (get_packet(&m))
		handle_msg(m.type, m.raw, m.len, timestamp);
}
