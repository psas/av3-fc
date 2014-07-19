#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include "elderberry/fcfutils.h"
#include "utilities/utils_sockets.h"
#include "utilities/psas_packet.h"
#include "utilities/net_addrs.h"
#include "rnh.h"



void rnh_raw_in(unsigned char *buffer, int unsigned len, unsigned char* timestamp) {
	char ID[4];
	switch(len){
	case sizeof(RNHAlarms):
		strncpy(ID, "RNHA", 4);
		break;
	case sizeof(RNHUmbdet):
		strncpy(ID, "RNHU", 4);
		break;
	case sizeof(RNHHealthData):
		strncpy(ID, "RNHH", 4);
		break;
	case sizeof(RNHPowerData):
		strncpy(ID, "RNHP", 4);
		break;
	default:
		return;
	}

	RNHMessage packet = {
		.timestamp={
			(uint8_t)timestamp[0], (uint8_t)timestamp[1],
			(uint8_t)timestamp[2], (uint8_t)timestamp[3],
			(uint8_t)timestamp[4], (uint8_t)timestamp[5]},
		.data_length=htons(len)
	};
	strncpy(packet.ID, ID, 4);
	memcpy(&packet.raw, buffer, len);

	rnh_data_out(&packet);
}

static void version_callback(struct pollfd *pfd){
	uint8_t buffer[50];

	int length = read(pfd->fd, buffer, sizeof(buffer));
	if(length < 0){
		perror("rnh version callback: read() failed");
	} else {
		rnh_version_out(buffer, length);
	}
	fcf_remove_fd(pfd->fd);
	close(pfd->fd);
}

void rnh_init(void){
	int s = socket(AF_INET, SOCK_STREAM, 0);
	if(s < 0){
		perror("Couldn't get rnh socket");
		return;
	}
	if(connect(s, RNH_RCI_ADDR, sizeof(struct sockaddr_in)) < 0){
		perror("rnh_init: connect() failed");
		close(s);
		return;
	}
	fcf_add_fd(s, POLLIN, version_callback);
	if(write(s, "#VERS\r\n", 7) < 0){
		perror("rnh_init: write failed");
	}

}
