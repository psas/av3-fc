/**
 *
 */
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>
#include "utils_sockets.h"
#include "fcfutils.h"
#include "net_addrs.h"
#include "adis.h"

void adis_getRawData_eth(unsigned char *buffer, int len) {

	// Build a ADIS packet
	// TODO: make timestamp
	if(len != sizeof(ADIS_packet)){
		ADIS_packet packet ={ .ID={buffer[0], buffer[1], buffer[2], buffer[3]},
							  .timestamp={0,0,0,0,0,0},
							  .data_length=buffer[10] << 8 | buffer[11]
		};
		unsigned char* data_section = &(buffer[12]);
		// Copy in data from socket
		memcpy(&packet.data, data_section, sizeof(ADIS16405_burst_data));

		// Send an ADIS packet to logger
		sendADISData(&packet);
	} //else log error?
}

void adis_init(){
}

void adis_final(){
}
