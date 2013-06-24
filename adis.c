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

#define DEVICE_NAME "ADIS"

void adis_getRawData_eth(char *buffer) {

	// Build a ADIS packet
	// TODO: make timestamp
	ADIS_packet packet ={ .ID="ADIS", .timestamp={0,0,0,0,0,0}, .data_length=htons(sizeof(ADIS16405_burst_data))};

	// Copy in data from socket
	memcpy(&packet.data, buffer, sizeof(ADIS16405_burst_data));

	// Send an ADIS packet to logger
	sendADISData(&packet);
}

void adis_init(){
}

void adis_final(){
}
