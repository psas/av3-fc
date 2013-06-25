/*
 *
 */
#include <string.h>
#include "adis.h"

void adis_raw_in(unsigned char *buffer, int len, unsigned char* timestamp) {
	if(len == sizeof(ADISMessage)){
		// Build message header
		ADISMessage packet ={ .ID={buffer[0], buffer[1], buffer[2], buffer[3]},
							  .timestamp={(uint8_t*)timestamp},
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
