
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>
#include "utils_sockets.h"
#include "fcfutils.h"
#include "net_addrs.h"
#include "adis.h"

#define DEVICE_NAME "ADIS"

// Buffer for reading in socket
static unsigned char buffer[1024];

/*
static void common_cb(const char * src, int fd) {

	// read ADIS data on socket
	int rc = readsocket(fd, buffer, sizeof(buffer));

	if (rc > 0) {

		// Build a ADIS packet
		// TODO: make timestamp
		ADIS_packet packet ={ .ID="ADIS", .timestamp={0,0,0,0,0,0}, .data_length=htons(sizeof(ADIS16405_burst_data))};

		// Copy in data from socket
		memcpy(&packet.data, buffer, sizeof(ADIS16405_burst_data));

		// Dump an ADIS packet
		sendADISData(&packet);
	}
}
*/

void adis_getRawData_eth(char *buffer) {
	ADIS_packet packet ={ .ID="ADIS", .timestamp={0,0,0,0,0,0}, .data_length=htons(sizeof(ADIS16405_burst_data))};
	sendADISData(&packet);
}


//active fd
//static void virtADIS_cb(struct pollfd *pfd){
//	common_cb(DEVICE_NAME, pfd->fd);
//}

int adis_init(){
	//int fd = getsocket(SENSOR_IP, ADIS_PORT_S, FC_LISTEN_PORT);
	//int rc = fcf_add_fd(fd, POLLIN, virtADIS_cb);
	return -1;
}

void adis_final(){
}


