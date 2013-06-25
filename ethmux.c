/*
 * ethmux.c
 *
 */
#include <unistd.h>
#include <netinet/in.h>
#include "utils_sockets.h"
#include "fcfutils.h"
#include "net_addrs.h"
#include "ethmux.h"

static unsigned char buffer[1024];

void demux(struct pollfd *pfd){
	struct sockaddr_in packet_info;
	socklen_t len = sizeof(packet_info);
	int bytes = readsocketfrom(pfd->fd, buffer, sizeof(buffer), (struct sockaddr *)&packet_info, &len);
	int port = ntohs(packet_info.sin_port);
	// TODO: make timestamp
	unsigned char timestamp[6] = {0,0,0,0,0,0};

	if(bytes > 0){
		switch(port){
		case ADIS_RX_PORT:
			demuxed_ADIS(buffer, bytes, timestamp);
			break;
		case ARM_PORT:
			demuxed_ARM(buffer, bytes, timestamp);
			break;
		case TEATHER_PORT:
			demuxed_LD(buffer, bytes, timestamp);
			break;
		case MPU_RX_PORT:
			demuxed_MPU(buffer, bytes, timestamp);
			break;
		case MPL_RX_PORT:
			demuxed_MPL(buffer, bytes, timestamp);
			break;
		default:
			break;
			// TODO: add a counter or debug logging of unknown ports
		}
	}
}

static int fd;
static int idx;

void ethmux_init(void){
	fd = getsocket(SENSOR_IP, ADIS_RX_PORT_S, FC_LISTEN_PORT);
	idx = fcf_add_fd(fd, POLLIN, demux);
}

void ethmux_final(void){
	//We really don't need to do this but just to be pedantic
	fcf_remove_fd(idx);
	close(fd);
}

