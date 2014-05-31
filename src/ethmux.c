#include <unistd.h>
#include <stdio.h>
#include <netinet/in.h>
#include <linux/net_tstamp.h>
#include <time.h>
#include <string.h>
#include <ev.h>
#include "utilities/utils_sockets.h"
#include "utilities/utils_time.h"
#include "utilities/net_addrs.h"
#include "ethmux.h"

static unsigned char buffer[ETH_MTU];

void demux(struct ev_loop *loop, ev_io *w, int revents){

	struct sockaddr_in packet_info;
	struct timespec ts;
	socklen_t len = sizeof(packet_info);
	int bytes = readsocketfromts(w->fd, buffer, sizeof(buffer), &packet_info, len, &ts);

	int port = ntohs(packet_info.sin_port);
	unsigned char timestamp[6];
	to_psas_time(&ts, timestamp);

	if(bytes > 0){
		switch(port){
		case ADIS_PORT:
			demuxed_ADIS(buffer, bytes, timestamp);
			break;
		case ARM_PORT:
			demuxed_ARM(buffer, bytes, timestamp);
			break;
		case TEATHER_PORT:
			demuxed_LD(buffer, bytes, timestamp);
			break;
		case MPU_PORT:
			demuxed_MPU(buffer, bytes, timestamp);
			break;
		case MPL_PORT:
			demuxed_MPL(buffer, bytes, timestamp);
			break;
		case RC_SERVO_ENABLE_PORT:
			demuxed_RC(buffer, bytes, timestamp);
			break;
		case RNH_BATTERY:
			demuxed_RNH(buffer, bytes, timestamp);
			break;
		case RNH_PORT:
			demuxed_RNHPORT(buffer, bytes, timestamp);
			break;
		case FCF_HEALTH_PORT:
			demuxed_FCFH(buffer, bytes, timestamp);
			break;
		default:
			break;
			// TODO: add a counter or debug logging of unknown ports
		}
	}
}

static int s;
static ev_io listen_watcher;

void ethmux_init(struct ev_loop * loop){
	s = timestamped_bound_udp_socket(FC_LISTEN_PORT);
	ev_io_init (&listen_watcher, demux, s, EV_READ);
	ev_io_start (loop, &listen_watcher);
}

void ethmux_final(void){
	close(s);
}

