/*
 *
 */
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include "adis.h"

void adis_raw_in(unsigned char *buffer, int len, unsigned char* timestamp) {
	if(len == sizeof(ADISMessage)){
		// Build message header
		ADISMessage packet ={
				.ID={"ADIS"},
				.timestamp={(uint8_t)timestamp[0], (uint8_t)timestamp[1],
						    (uint8_t)timestamp[2], (uint8_t)timestamp[3],
						    (uint8_t)timestamp[4], (uint8_t)timestamp[5]},
				.data_length=buffer[10] << 8 | buffer[11]
		};
		// Copy in data from socket
		unsigned char* data_section = &(buffer[12]);
		memcpy(&packet.data, data_section, sizeof(ADIS16405BurstData));

		// Send data out
		adis_data_out(&packet);
	}
	//TODO: else log error?
}
