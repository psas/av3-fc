#include <unistd.h>
#include <stdio.h>
#include <netinet/in.h>
#include <linux/net_tstamp.h>
#include <time.h>
#include <string.h>
#include "devices/mpl.h"
#include "devices/rnh.h"
#include "elderberry/fcfutils.h"
#include "utilities/net_addrs.h"
#include "utilities/psas_packet.h"
#include "utilities/utils_sockets.h"
#include "utilities/utils_time.h"
#include "ethmux.h"

static uint8_t buffer[ETH_MTU];

struct demux_type {
	const char ID[4];
	const uint16_t data_length;
	void (*const handler)(const char ID[4], uint8_t timestamp[6], uint16_t data_length, void *buffer);
	uint32_t next_sequence;
};

static int is_expected_length(struct demux_type *type, unsigned int len) {
	if (type->data_length)
		return len == sizeof(uint32_t) + type->data_length;
	return len >= sizeof(uint32_t) && len <= UINT16_MAX - sizeof(uint32_t);
}

//FIXME: move to psas_packet
struct seqerror {
	struct {
		uint16_t port;
		uint32_t expected;
		uint32_t received;
	} meta;
	uint8_t buf[1024];
} __attribute__((packed));

void format_sequenced_error(unsigned short port, uint8_t * buffer, unsigned int len, uint8_t * timestamp, uint32_t expected, uint32_t received) {
	struct seqerror err = {
		.meta = {
			.port = htons(port),
			.expected = htonl(expected),
			.received = htonl(received),
		},
	};

	if (len > sizeof err.buf)
		len = sizeof err.buf;

	memcpy(err.buf, buffer, len);

	sequenced_error("SEQE", timestamp, sizeof err.meta + len, &err);
}

void sequenced_receive(unsigned short port, uint8_t * buffer, unsigned int len, uint8_t* timestamp, struct demux_type *type) {
	if (!is_expected_length(type, len)) {
		format_sequenced_error(port, buffer, len, timestamp, 0, 0);
		return;
	}

	uint32_t rcvseq = ntohl(*(uint32_t*)buffer);
	buffer += sizeof(uint32_t);
	len -= sizeof(uint32_t);

	if (rcvseq < type->next_sequence) {
		format_sequenced_error(port, buffer, len, timestamp, type->next_sequence, rcvseq);
	}
	if (rcvseq > type->next_sequence) {
		format_sequenced_error(port, NULL, 0, timestamp, type->next_sequence, rcvseq);
		type->handler(type->ID, timestamp, len, buffer);
	}
	if (rcvseq == type->next_sequence) {
		type->handler(type->ID, timestamp, len, buffer);
	}

	type->next_sequence = rcvseq + 1;
}

void demux(struct pollfd *pfd){
	static struct demux_type seq_ADIS = { "ADIS", sizeof(ADIS16405Data), demuxed_ADIS };
	static struct demux_type seq_MPU = { "MPU9", UINT16_MAX, demuxed_MPU };
	static struct demux_type seq_MPL = { "MPL3", sizeof(MPLData), demuxed_MPL };
	static struct demux_type seq_RNH = { "RNHH", sizeof(RNHHealthData), demuxed_RNH };
	static struct demux_type seq_RNHPORT = { "RNHP", sizeof(RNHPowerData), demuxed_RNH };
	static struct demux_type seq_RNHALARM = { "RNHA", sizeof(RNHAlarms), demuxed_RNH };
	static struct demux_type seq_RNHUMBDET = { "RNHU", sizeof(RNHUmbdet), demuxed_RNH };
	static struct demux_type seq_FCFH = { "FCFH", sizeof(FCFHealthData), demuxed_FCFH };
	static struct demux_type seq_JGPS = { "JGPS", 0, demuxed_JGPS };
	static struct demux_type seq_GPS_COTS = { "V8BS", 0, demuxed_COTS };
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
			sequenced_receive(port, buffer, bytes, timestamp, &seq_ADIS);
			break;
		case ARM_PORT:
			demuxed_ARM(buffer, bytes, timestamp);
			break;
		case MPU_PORT:
			sequenced_receive(port, buffer, bytes, timestamp, &seq_MPU);
			break;
		case MPL_PORT:
			sequenced_receive(port, buffer, bytes, timestamp, &seq_MPL);
			break;
		case RC_SERVO_ENABLE_PORT:
			demuxed_RC(buffer, bytes, timestamp);
			break;
		case RNH_BATTERY:
			sequenced_receive(port, buffer, bytes, timestamp, &seq_RNH);
			break;
		case RNH_PORT:
			sequenced_receive(port, buffer, bytes, timestamp, &seq_RNHPORT);
			break;
		case RNH_ALARM:
			sequenced_receive(port, buffer, bytes, timestamp, &seq_RNHALARM);
			break;
		case RNH_UMBDET:
			sequenced_receive(port, buffer, bytes, timestamp, &seq_RNHUMBDET);
			break;
		case FCF_HEALTH_PORT:
			sequenced_receive(port, buffer, bytes, timestamp, &seq_FCFH);
			break;
		case JGPS_PORT:
			sequenced_receive(port, buffer, bytes, timestamp, &seq_JGPS);
			break;
		case GPS_COTS:
			sequenced_receive(port, buffer, bytes, timestamp, &seq_GPS_COTS);
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

