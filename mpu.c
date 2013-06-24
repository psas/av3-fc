/*
 * mpu.c
 *
 */

#include <string.h>
#include <stdint.h>
#include "mpu.h"

void mpu_getRawData_eth(unsigned char *buffer, int len, unsigned char* timestamp) {

	// Build an MPU packet
	if(len != sizeof(MPU_packet)){
		MPU_packet packet ={ .ID={buffer[0], buffer[1], buffer[2], buffer[3]},
							  .timestamp={(uint8_t*)timestamp},
							  .data_length=buffer[10] << 8 | buffer[11]
		};
//		unsigned char* data_section = &(buffer[12]);
//		// Copy in data from socket
//		memcpy(&packet.data, data_section, sizeof(ADIS16405_burst_data));

		packet.data = buffer;
		// Send an MPU packet to logger
		sendMPUData(&packet);
	} //else log error?
}
