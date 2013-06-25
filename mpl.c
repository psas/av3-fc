/*
 * mpl.c
 *
 */

#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include "mpl.h"

void mpl_raw_in(unsigned char *buffer, int len, unsigned char* timestamp) {
	if(len == sizeof(MPLMessage)){
		// Build message header
		MPLMessage packet ={
				.ID={"MPL3"},
				.timestamp={(uint8_t)timestamp[0], (uint8_t)timestamp[1],
						    (uint8_t)timestamp[2], (uint8_t)timestamp[3],
						    (uint8_t)timestamp[4], (uint8_t)timestamp[5]},
				.data_length=buffer[10] << 8 | buffer[11]
		};
		// Copy in data from socket
		packet.data = buffer;
//		unsigned char* data_section = &(buffer[12]);
//		memcpy(&packet.data, data_section, sizeof(MPL_burst_data));

		// Send data out
		mpl_data_out(&packet);
	}
	// TODO: else log error?
}
