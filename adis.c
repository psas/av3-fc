/*
 *
 */
#include <string.h>
#include "adis.h"

void adis_getRawData_eth(unsigned char *buffer, int len, unsigned char* timestamp) {

	// Build a ADIS packet
	if(len != sizeof(ADIS_packet)){
		ADIS_packet packet ={ .ID={buffer[0], buffer[1], buffer[2], buffer[3]},
							  .timestamp={(uint8_t*)timestamp},
							  .data_length=buffer[10] << 8 | buffer[11]
		};
		unsigned char* data_section = &(buffer[12]);
		// Copy in data from socket
		memcpy(&packet.data, data_section, sizeof(ADIS16405_burst_data));

		// Send an ADIS packet to logger
		sendADISData(&packet);
	} //else log error?
}
