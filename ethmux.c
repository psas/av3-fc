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

static unsigned char buffer[1024];

void ethmux_cb(struct pollfd *pfd){

	struct sockaddr_in packet_info;
	int len = sizeof(packet_info);
	int rc = readsocketfrom(pfd->fd, buffer, sizeof(buffer), &packet_info, (socklen_t *)&len);
	int port = ntohs(packet_info.sin_port);

	// Filter ports to modules
	if (port == ADIS_RX_PORT)
		adis_getRawData_eth(buffer);

}

int ethmux_init(){
	int fd = getsocket(SENSOR_IP, ADIS_RX_PORT_S, FC_LISTEN_PORT);
	int rc = fcf_add_fd(fd, POLLIN, ethmux_cb);
	return rc;
}

void ethmux_final(){

}
