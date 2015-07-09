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

static void *mem2chr(uint8_t *s, uint8_t c1, uint8_t c2, size_t len)
{
	uint8_t *out = memchr(s, c1, len-1);
	while (out != NULL)
	{
		if (out[1] == c2)
			return out;
		len -= out+1 - s;
		s = out+1;
		out = memchr(s, c1, len-1);
	}
	return out;
}

/* Venus binary data is big-endian: must swap for interpretation */
static inline uint16_t swapShort(uint16_t s)
{
	return (s >> 8) | (s << 8);
}

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
	uint8_t *frame = mem2chr(buf, 0xA0, 0xA1, cur - buf);
	if (frame != buf)
	{
		if (frame == NULL)
			printf("Frame marker not found.\n");
		printf("Dropped %ld bytes of data from GPS stream.\n", cur - buf);
		//dump(buf, cur-buf);
	}
	while (frame != NULL)
	{
		uint16_t packet_len;
		if (frame + 2+2 > cur)
		{
			/* not a full packet, move remaining data to start of buf */
			memmove(buf, frame, cur-frame);
			cur -= frame-buf;
			return;
		}
		memcpy(&packet_len, frame+2, 2);
		packet_len = swapShort(packet_len);
		
		/* sanity check */
		if (packet_len > 163)	// max len of 0xDE: 3 + 10 bytes * 16 sats
		{
			perror("Bad GPS packet length.");
			/* packet_len corrupt, resync on packet terminator \r\n */
			frame = mem2chr(frame, '\r', '\n', cur - frame);
			if (frame == NULL)
				return;
			frame = mem2chr(frame, 0xA0, 0xA1, cur - frame);
			continue;
		}
		if (frame + 2+2 + packet_len +1+2 > cur)
		{
			/* not a full packet, move remaining data to start of buf */
			memmove(buf, frame, cur-frame);
			cur -= frame-buf;
			return;
		}
		if (chexxor(frame+4, packet_len) == frame[4+packet_len])
		{
			/* full packet, transmit ID "V8XX" where XX is the hex packet ID */
			char ID[5];
			sprintf(ID, "V8%02X", frame[4]);
			gps_data_out(ID, timestamp, packet_len-1, frame+5);
		}
		else
			perror("Bad GPS packet checksum.");
		frame += packet_len + 7;
		if (frame + 2+2 > cur)
		{
			/* not a full packet, move remaining data to start of buf */
			memmove(buf, frame, cur-frame);
			cur -= frame-buf;
			return;
		}
		frame = mem2chr(frame, 0xA0, 0xA1, cur - frame);
	}
	/* no more packets, start over */
	printf("Dropped %ld bytes of data from GPS stream.\n", cur - buf);
	//dump(buf, cur-buf);
	cur = buf;
}

/**
 * Recieves a COTS GPS message from the network
 */
void cots_raw_in(const char ID[4], uint8_t timestamp[6], uint16_t data_length, void *buffer)
{
	if (cur + data_length >= buf + sizeof(buf))
	{
		perror("GPS buffer overrun!");
		cur = buf;
	}
	memcpy(cur, buffer, data_length);
	cur += data_length;
	send_venus_messages(timestamp);

	// TODO: instead of logging the raw byte stream, identify frame boundaries and emit only valid frames.
	//gps_data_out(ID, timestamp, data_length, buffer);
}

#ifdef TESTING

struct packetA8 {
	uint8_t head[2];
	uint16_t len;		// NOTE: big-endian
	uint8_t ID;
	uint8_t seq;		// for testing, add a sequence # to check dropped packets
	uint8_t data[57];
	uint8_t checksum;
	uint8_t tail[2];
} __attribute__((packed)) pA8 = { { 0xA0, 0xA1 }, 59, 0xA8, 0, { }, 0, { '\r', '\n' } };

uint8_t input[0x10000];

int main(int argc, char *argv[])
{
	char ID[5] = "GPSV";	//???
	uint8_t timestamp[6] = { 0,0,0,0,0,0 } ;
	size_t chunk = 100;
	
	pA8.len = swapShort(pA8.len);

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
