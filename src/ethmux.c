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

typedef void (*demux_handler)(unsigned char* buffer, unsigned int bytes, unsigned char* timestamp);

static unsigned char buffer[ETH_MTU];
static unsigned long seq_ADIS;
static unsigned long seq_ARM;
static unsigned long seq_LD;
static unsigned long seq_MPU;
static unsigned long seq_MPL;
static unsigned long seq_RC;
static unsigned long seq_RNH;
static unsigned long seq_RNHPORT;
static unsigned long seq_FCFH;

static void sequenced_error(int port, unsigned char* buffer, unsigned int bytes, unsigned char* timestamp, enum SeqError error, unsigned long expected, unsigned long rcvd);
static void sequenced_receive(int port, unsigned char* buffer, unsigned int bytes, unsigned char* timestamp, unsigned long* seq, demux_handler handler);

void sequenced_error(int port, unsigned char* buffer, unsigned int bytes, unsigned char* timestamp, enum SeqError error, unsigned long expected, unsigned long rcvd) {
}

void sequenced_receive(int port, unsigned char* buffer, unsigned int bytes, unsigned char* timestamp, unsigned long* seq, demux_handler handler) {
    if (bytes < sizeof(unsigned long)) {
        sequenced_error(port, buffer, bytes, timestamp, SEQ_noseq, 0, 0);
    } else {
        unsigned long   rcvseq;

        rcvseq = ntohl(*(unsigned long*)buffer);

        if (rcvseq < *seq) {
            sequenced_error(port, buffer, bytes, timestamp, SEQ_backward, *seq, rcvseq);

            *seq = rcvseq + 1;
        } else {
            if (rcvseq > *seq) {
                sequenced_error(port, buffer, bytes, timestamp, SEQ_skip, *seq, rcvseq);
            }

            *seq = rcvseq + 1;

            handler(buffer + sizeof(unsigned long), bytes - sizeof(unsigned long), timestamp);
        }
    }
}

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
        case ADIS_PORT:
            sequenced_receive(port, buffer, bytes, timestamp, &seq_ADIS, demuxed_ADIS);
            break;
        case ARM_PORT:
            sequenced_receive(port, buffer, bytes, timestamp, &seq_ARM, demuxed_ARM);
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
static int idx;

void ethmux_init(void){
    fd = timestamped_bound_udp_socket(FC_LISTEN_PORT);
    if(fd < 0){
        return;
    }
    idx = fcf_add_fd(fd, POLLIN, demux);
}

void ethmux_final(void){
    //We really don't need to do this but just to be pedantic
    fcf_remove_fd(idx);
    close(fd);
}

