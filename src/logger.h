/*
 * Handles handles formatting and logging messages to disk and over WiFi
 */

#ifndef _LOGGER_H_
#define _LOGGER_H_

#include "gps.h"
#include "rollcontrol.h"

typedef struct{
	char     ID[4];
	uint8_t  timestamp[6];
	uint16_t data_length;
} __attribute__((packed)) message_header;

void logger_init(void); // [miml:init]
void logger_final(void); // [miml:final]

void log_write_disk_only(const char ID[4], const uint8_t timestamp[6], uint16_t data_length, const void *data);
void log_write(const char ID[4], const uint8_t timestamp[6], uint16_t data_length, const void *data);
void log_receive_state(VSTEMessage *); // [miml:receiver]
void log_receive_gps(V6NAMessage*);
void log_receive_rc(ROLLMessage*);
void log_receive_rnh_version(uint8_t * buffer, unsigned int length);
void log_receive_arm(const char*);


#endif
