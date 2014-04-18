#include <unistd.h>
#include <stdio.h>
#include <netinet/in.h>
#include <linux/net_tstamp.h>
#include <time.h>
#include <string.h>
#include "elderberry/fcfutils.h"
#include "utilities/utils_sockets.h"
#include "utilities/utils_time.h"
#include "utilities/net_addrs.h"
#include "ethmux.h"

static unsigned char buffer[1024]; // TODO: packet len

void demux(struct pollfd *pfd){

	struct sockaddr_in packet_info;
	struct timespec ts;
	socklen_t len = sizeof(packet_info);
	int bytes = readsocketfromts(pfd->fd, buffer, sizeof(buffer), &packet_info, len, &ts);

	int port = ntohs(packet_info.sin_port);
	unsigned char timestamp[6];
	to_psas_time(&ts, timestamp);

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
		case RC_SERVO_ENABLE_PORT:
			demuxed_RC(buffer, bytes, timestamp);
			break;
		case RNH_BATTERY_PORT:
			demuxed_RNH(buffer, bytes, timestamp);
			break;
        case RNH_PORT_PORT:
            demuxed_RNHPORT(buffer, bytes, timestamp);
            break;
		default:
			break;
			// TODO: add a counter or debug logging of unknown ports
		}
	}
}

static int fd;
static int idx;

/*
 * because of preprocessor shenanigans, macro defined constants need to pass
 * through two layers of macro function to correctly stringify.
 */
#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)

void ethmux_init(void){
	fd = getsocket(FC_IP, STRINGIFY(FC_LISTEN_PORT), FC_LISTEN_PORT);
	idx = fcf_add_fd(fd, POLLIN, demux);
}

void ethmux_final(void){
	//We really don't need to do this but just to be pedantic
	fcf_remove_fd(idx);
	close(fd);
}

