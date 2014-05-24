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

typedef struct{
	char     ID[4];
	uint8_t  timestamp[6];
	uint16_t data_length;
} __attribute__((packed)) message_header;

void logger_init(void); // [miml:init]
void logger_final(void); // [miml:final]

extern void log_receive_adis(ADISMessage *); // [miml:receiver]
extern void log_receive_gps(GPSMessage*);
extern void log_receive_mpu(MPUMessage*);
extern void log_receive_mpl(MPLMessage*);
extern void log_receive_rc(RollServoMessage*);
extern void log_receive_rnh(unsigned char *buffer, int unsigned len, unsigned char* timestamp);
extern void log_receive_rnhport(unsigned char *buffer, int unsigned len, unsigned char* timestamp);
extern void log_receive_fcfh(unsigned char *buffer, int unsigned len, unsigned char* timestamp);

extern void log_receive_arm(const char*);


#endif
