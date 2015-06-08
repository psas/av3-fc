/*
 * Handles handles formatting and logging messages to disk and over WiFi
 */

#ifndef _LOGGER_H_
#define _LOGGER_H_

#include "adis.h"
#include "gps.h"
#include "mpl.h"
#include "mpu.h"
#include "rollcontrol.h"
#include "rnh.h"

typedef struct{
	char     ID[4];
	uint8_t  timestamp[6];
	uint16_t data_length;
} __attribute__((packed)) message_header;

void logger_init(void); // [miml:init]
void logger_final(void); // [miml:final]

void log_receive_adis(ADISMessage *); // [miml:receiver]
void log_receive_state(VSTEMessage *); // [miml:receiver]
void log_receive_gps(GPSMessage*);
void log_receive_mpu(MPUMessage*);
void log_receive_mpl(MPLMessage*);
void log_receive_rc(RollServoMessage*);
void log_receive_rnh(RNHMessage * packet);
void log_receive_rnh_version(uint8_t * buffer, unsigned int length);
void log_receive_fcfh(unsigned char *buffer, int unsigned len, unsigned char* timestamp);
void log_receive_arm(const char*);
void log_receive_seqpacket_err(unsigned short port, uint8_t * buffer, unsigned int len, uint8_t * timestamp, uint32_t expected, uint32_t received);


#endif
