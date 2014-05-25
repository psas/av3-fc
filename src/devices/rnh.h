#ifndef RNH_H_
#define RNH_H_
#include "psas_packet.h"

void rnhh_raw_in(unsigned char *buffer, int unsigned len, unsigned char* timestamp);
void rnhp_raw_in(unsigned char *buffer, int unsigned len, unsigned char* timestamp);
void rnhh_data_out(RNHHMessage * message);
void rnhp_data_out(RNHPMessage * message);
void rnh_version_out(uint8_t * buffer, unsigned int length);
void rnh_init(void);
#endif
