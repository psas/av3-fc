/*
 * ethmux.c
 *
 */
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include "utils_sockets.h"
#include "fcfutils.h"
#include "net_addrs.h"
#include "ethmux.h"

static unsigned char buffer[1024];

void ethmux_cb(struct pollfd *pfd){

	struct sockaddr_in packet_info;
	socklen_t len = sizeof(packet_info);
	int rc = readsocketfrom(pfd->fd, buffer, sizeof(buffer), (struct sockaddr *)&packet_info, &len);
	int port = ntohs(packet_info.sin_port);

	// Filter ports to modules
	if(rc > 0){
		switch(port){
		case ADIS_RX_PORT:
			sendADISPacket(buffer, rc);
			break;
		case ARM_PORT:
			sendARMPacket(buffer, rc);
			break;
		default:
			printf("unrec");
		}
	}
}

int ethmux_init(){
	int fd = getsocket(SENSOR_IP, ADIS_RX_PORT_S, FC_LISTEN_PORT);
	int rc = fcf_add_fd(fd, POLLIN, ethmux_cb);
	return rc;
}

void ethmux_final(){

}
