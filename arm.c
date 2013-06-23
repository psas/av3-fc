/*
 * arm.c
 *
 *  Created on: Jun 22, 2013
 *      Author: theo
 */

#include "arm.h"
#include "net_addrs.h"
#include "utils_sockets.h"
#include "fcfutils.h"

static unsigned char buffer[100];

static void arm_signal_cb(struct pollfd *pfd){
	int rc = readsocket(pfd->fd, buffer, sizeof(buffer));
	if(rc > 0){
		sendARMData((char *)buffer);
	}
}

int arm_init(){
	int fd = getsocket(ARM_IP, ARM_PORT_S, FC_LISTEN_PORT);
	int rc = fcf_add_fd(fd, POLLIN, arm_signal_cb);
	return rc;
}
void arm_final(){
	return;
}

void arm_getPositionData_adis(ADIS_packet * data){
	return;
}
void arm_getPositionData_gps(GPS_packet * data){
	return;
}
