#ifndef RNH_H_
#define RNH_H_
#include <ev.h>
#include "psas_packet.h"

void MIML_RECEIVER rnhh_raw_in(unsigned char *buffer, int unsigned len, unsigned char* timestamp);
void MIML_RECEIVER rnhp_raw_in(unsigned char *buffer, int unsigned len, unsigned char* timestamp);
void MIML_SENDER rnhh_data_out(RNHHMessage * message);
void MIML_SENDER rnhp_data_out(RNHPMessage * message);
void MIML_SENDER rnh_version_out(uint8_t * buffer, unsigned int length);
void MIML_INIT rnh_init(struct ev_loop * loop);
#endif
