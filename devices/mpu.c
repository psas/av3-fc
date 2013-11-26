/*
 * mpu.c
 *
 */

#include <string.h>
#include <stdint.h>
#include "mpu.h"

void mpu_raw_in(unsigned char *buffer, unsigned int len, unsigned char* timestamp) {
	if(len == sizeof(MPUMessage)){
		// Build message header
		MPUMessage packet ={
				.ID={"MPU9"},
				.timestamp={(uint8_t)timestamp[0], (uint8_t)timestamp[1],
						    (uint8_t)timestamp[2], (uint8_t)timestamp[3],
						    (uint8_t)timestamp[4], (uint8_t)timestamp[5]},
				.data_length=buffer[10] | buffer[11] << 8
		};
		// Copy in data from socket
		packet.data = buffer;
//		unsigned char* data_section = &(buffer[12]);
//		memcpy(&packet.data, data_section, sizeof(MPL_burst_data));

		// Send data out
		mpu_data_out(&packet);
	}
	// TODO: else log error?
}
