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

static uint8_t buffer[ETH_MTU];

typedef void (*demux_handler)(uint8_t * buffer, unsigned int len, uint8_t * timestamp);

void sequenced_receive(unsigned short port, uint8_t * buffer, unsigned int len, uint8_t* timestamp, uint32_t * seq, demux_handler handler) {
	if (len < sizeof(uint32_t)) {
		sequenced_error(port, buffer, len, timestamp, 0, 0);
		return;
	}

	uint32_t rcvseq = ntohl(*(uint32_t*)buffer);
	buffer += sizeof(uint32_t);
	len -= sizeof(uint32_t);

	if (rcvseq < *seq) {
		sequenced_error(port, buffer, len, timestamp, *seq, rcvseq);
	}
	if (rcvseq > *seq) {
		sequenced_error(port, NULL, 0, timestamp, *seq, rcvseq);
		handler(buffer, len, timestamp);
	}
	if (rcvseq == *seq) {
		handler(buffer, len, timestamp);
	}

	*seq = rcvseq + 1;
}

void demux(struct pollfd *pfd){
	static uint32_t seq_ADIS = 0;
	static uint32_t seq_LD = 0;
	static uint32_t seq_MPU = 0;
	static uint32_t seq_MPL = 0;
	static uint32_t seq_RC = 0;
	static uint32_t seq_RNH = 0;
	static uint32_t seq_RNHPORT = 0;
	static uint32_t seq_FCFH = 0;
	struct sockaddr_in packet_info;
	struct timespec ts;
	socklen_t len = sizeof(packet_info);
	int bytes = readsocketfromts(pfd->fd, buffer, sizeof(buffer), &packet_info, len, &ts);

	unsigned short port = ntohs(packet_info.sin_port);
	uint8_t timestamp[6];
	to_psas_time(&ts, timestamp);

	if(bytes > 0){
		switch(port){
		case ADIS_PORT:
			sequenced_receive(port, buffer, bytes, timestamp, &seq_ADIS, demuxed_ADIS);
			break;
		case ARM_PORT:
            demuxed_ARM(buffer, bytes, timestamp);
			break;
		case TEATHER_PORT:
			sequenced_receive(port, buffer, bytes, timestamp, &seq_LD, demuxed_LD);
			break;
		case MPU_PORT:
			sequenced_receive(port, buffer, bytes, timestamp, &seq_MPU, demuxed_MPU);
			break;
		case MPL_PORT:
			sequenced_receive(port, buffer, bytes, timestamp, &seq_MPL, demuxed_MPL);
			break;
		case RC_SERVO_ENABLE_PORT:
			sequenced_receive(port, buffer, bytes, timestamp, &seq_RC, demuxed_RC);
			break;
		case RNH_BATTERY:
			sequenced_receive(port, buffer, bytes, timestamp, &seq_RNH, demuxed_RNH);
			break;
		case RNH_PORT:
			sequenced_receive(port, buffer, bytes, timestamp, &seq_RNHPORT, demuxed_RNHPORT);
			break;
		case FCF_HEALTH_PORT:
			sequenced_receive(port, buffer, bytes, timestamp, &seq_FCFH, demuxed_FCFH);
			break;
		default:
			break;
			// TODO: add a counter or debug logging of unknown ports
		}
	}
}

static int fd;

void ethmux_init(void){
	fd = timestamped_bound_udp_socket(FC_LISTEN_PORT);
	if(fd < 0){
		return;
	}
	fcf_add_fd(fd, POLLIN, demux);
}

void ethmux_final(void){
	//We really don't need to do this but just to be pedantic
	fcf_remove_fd(fd);
	close(fd);
}

