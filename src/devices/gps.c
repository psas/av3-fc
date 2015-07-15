#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#ifndef TESTING
#include "gps.h"

#else

#include <stdint.h>
void gps_data_out(const char ID[4], uint8_t timestamp[6], uint16_t data_length, void *buffer)
{
	uint8_t seq = *(uint8_t *)buffer;
	printf("GPS packet: %s#%u @ %u len %u\n", ID, seq, timestamp[5], data_length);
}
void dump(uint8_t *buf, size_t len)
{
	for (size_t i=0; i<len; ++i)
	{
		printf("%02X ", *buf++);
		if ((i & 0xf) == 0xf)
			putchar('\n');
	}
	putchar('\n');
}
#endif

static uint8_t buf[512];	/* at least 2x longer than longest GPS message */
static uint8_t *cur = buf;

static uint8_t chexxor(uint8_t *data, size_t len)
{
	uint8_t ret = 0;
	while (len--)
		ret ^= *data++;
	return ret;
}

/*
 * Identify frame boundaries for Venus messages and emit only valid messages.
 *  Frames start with 0xA0 0xA1, then 16-bit big-endian packet length.
 *  The packet consists of an ID byte and data.
 *  Following the packet is an XOR checksum byte of ID and body, then \r\n.
 *
 * Sent data is extracted from the frame, and given an ID that encodes its packet type.
 */
static void send_venus_messages(uint8_t timestamp[6])
{
	uint8_t *frame = buf;
	while (frame < cur)
	{
		uint8_t *start = frame;
		frame = memchr(start, 0xA0, cur - start);
		if (frame == NULL) // reached end of buffer
		{
			fprintf(stderr, "No packet found. Dropped %ld bytes of data from GPS stream.\n", cur - start);
			//dump(start, cur - start);
			cur = buf;
			return;
		}
		if (frame != start)
		{
			fprintf(stderr, "Dropped %ld bytes of data from GPS stream.\n", frame - start);
			//dump(start, frame - start);
		}

		/* header + trailer is 7 bytes */
		if (frame + 7 > cur)
			break; // wait for more data

		if (frame[1] != 0xA1)
		{
			frame += 1; // skip A0
			continue;
		}

		uint16_t packet_len = (frame[2] << 8) | frame[3];
		
		/* sanity check */
		if (packet_len > sizeof buf / 2)
		{
			fprintf(stderr, "GPS packet with ID %02X too long: %u bytes.\n", frame[4], packet_len);
			frame += 2; // skip A0A1
			continue;
		}

		if (frame + 7 + packet_len > cur)
			break; // wait for more data

		if (memcmp(frame + 5 + packet_len, "\r\n", 2))
		{
			fprintf(stderr, "GPS packet with ID %02X does not end with frame trailer.\n", frame[4]);
			frame += 2; // skip A0A1
			continue;
		}

		if (chexxor(frame+4, packet_len) != frame[4+packet_len])
		{
			fprintf(stderr, "Bad GPS packet checksum for ID %02X.\n", frame[4]);
			frame += 2; // skip A0A1
			continue;
		}

		/* full packet, transmit ID "V8XX" where XX is the hex packet ID */
		char ID[5];
		sprintf(ID, "V8%02X", frame[4]);
		gps_data_out(ID, timestamp, packet_len-1, frame+5);

		frame += packet_len + 7;
	}

	/* not a full packet, move remaining data to start of buf */
	memmove(buf, frame, cur - frame);
	cur -= frame - buf;
}

/**
 * Recieves a COTS GPS message from the network
 */
void cots_raw_in(const char ID[4], uint8_t timestamp[6], uint16_t data_length, void *buffer)
{
	if (cur + data_length >= buf + sizeof(buf))
	{
		fprintf(stderr, "GPS buffer overrun! Dropped %ld bytes of data from GPS stream.\n", cur - buf);
		cur = buf;
	}
	memcpy(cur, buffer, data_length);
	cur += data_length;
	send_venus_messages(timestamp);

	// TODO: instead of logging the raw byte stream, identify frame boundaries and emit only valid frames.
	//gps_data_out(ID, timestamp, data_length, buffer);
}

#ifdef TESTING
#include <arpa/inet.h>

struct packetA8 {
	uint8_t head[2];
	uint16_t len;		// NOTE: big-endian
	uint8_t ID;
	uint8_t seq;		// for testing, add a sequence # to check dropped packets
	uint8_t data[57];
	uint8_t checksum;
	uint8_t tail[2];
} __attribute__((packed)) pA8 = { { 0xA0, 0xA1 }, 0, 0xA8, 0, { }, 0, { '\r', '\n' } };

uint8_t input[0x10000];

int main(int argc, char *argv[])
{
	char ID[5] = "GPSV";	//???
	uint8_t timestamp[6] = { 0,0,0,0,0,0 } ;
	size_t chunk = 100;

	pA8.len = htons(59);

	/* fill input */
	uint8_t *p = input;
	while (p + sizeof(pA8) < input + sizeof(input))
	{
		/* random data payload */
		for (size_t i=0; i<sizeof(pA8.data); i++)
			pA8.data[i] = random() & 0xff;
		pA8.checksum = chexxor(&pA8.ID, sizeof(pA8.data)+1+1);
		memcpy(p, &pA8, sizeof(pA8));
		p += sizeof(pA8);
		pA8.seq++;
	}
	
	/* drain input, random chunk lengths of about 100 bytes */
	p = input;
	while (p + chunk < input + sizeof(input))
	{
		cots_raw_in(ID, timestamp, chunk, p);
		p += chunk;
		timestamp[5]++;
		chunk = 90 + random() % 20;
	}

	return 0;
}
#endif
