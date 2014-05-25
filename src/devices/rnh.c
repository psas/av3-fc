#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include "elderberry/fcfutils.h"
#include "utilities/utils_sockets.h"
#include "utilities/psas_packet.h"
#include "utilities/net_addrs.h"
#include "rnh.h"

void rnhh_raw_in(unsigned char *buffer, int unsigned len, unsigned char* timestamp) {
	if (len == sizeof(RNHHealthData)) {

		RNHHMessage packet = {
			.ID={"RNHH"},
			.timestamp={
				(uint8_t)timestamp[0], (uint8_t)timestamp[1],
				(uint8_t)timestamp[2], (uint8_t)timestamp[3],
				(uint8_t)timestamp[4], (uint8_t)timestamp[5]},
			.data_length=htons(sizeof(RNHHealthData))
		};
		// Copy in data from socket
		memcpy(&packet.data, buffer, sizeof(RNHHealthData));

		rnhh_data_out(&packet);
	}
}

void rnhp_raw_in(unsigned char *buffer, int unsigned len, unsigned char* timestamp) {
	if (len == sizeof(RNHPowerData)) {

		RNHPMessage packet = {
			.ID={"RNHP"},
			.timestamp={
				(uint8_t)timestamp[0], (uint8_t)timestamp[1],
				(uint8_t)timestamp[2], (uint8_t)timestamp[3],
				(uint8_t)timestamp[4], (uint8_t)timestamp[5]},
			.data_length=htons(sizeof(RNHPowerData))
		};
		// Copy in data from socket
		memcpy(&packet.data, buffer, sizeof(RNHPowerData));

		rnhp_data_out(&packet);
	}
}

int s, idx;
uint8_t buffer[50];
static void version_callback(struct pollfd *pfd){

	int length = read(pfd->fd, buffer, sizeof(buffer));
	if(length < 0){
		perror("rnh version callback: read() failed");
	} else {
		rnh_version_out(buffer, length);
	}
	fcf_remove_fd(idx);
	close(s);
}

void rnh_init(void){
	s = bound_udp_socket(0);
	if(connect(s, RNH_RCI_ADDR, sizeof(struct sockaddr_in)) < 0){
		perror("rnh_init: connect() failed");
		close(s);
	}
	idx = fcf_add_fd(s, POLLIN, version_callback);
	if(write(s, "#VERS", 5) < 0){
		perror("rnh_init: write failed");
	}

}
