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

void MIML_INIT logger_init(void);
void MIML_FINAL logger_final(void);

void MIML_RECEIVER log_receive_adis(ADISMessage *);
void MIML_RECEIVER log_receive_gps(GPSMessage*);
void MIML_RECEIVER log_receive_mpu(MPUMessage*);
void MIML_RECEIVER log_receive_mpl(MPLMessage*);
void MIML_RECEIVER log_receive_rc(RollServoMessage*);
void MIML_RECEIVER log_receive_rnhh(RNHHMessage * packet);
void MIML_RECEIVER log_receive_rnhp(RNHPMessage * packet);
void MIML_RECEIVER log_receive_rnh_version(uint8_t * buffer, unsigned int length);
void MIML_RECEIVER log_receive_fcfh(unsigned char *buffer, int unsigned len, unsigned char* timestamp);
void MIML_RECEIVER log_receive_arm(const char*);



#endif
