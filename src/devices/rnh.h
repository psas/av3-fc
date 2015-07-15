#ifndef RNH_H_
#define RNH_H_
#include "psas_packet.h"

typedef struct {
	char ID[4];
	uint8_t timestamp[6];
	uint16_t data_length;
	uint8_t raw[50];
} __attribute__((packed)) RNHMessage;


typedef struct {
	uint16_t alarm1;
	uint16_t alarm2;
	uint16_t alarm3;
} __attribute__((packed)) RNHAlarms;

typedef struct {
	uint8_t detect;
} __attribute__((packed)) RNHUmbdet;

void rnh_version_out(uint8_t * buffer, unsigned int length);
void rnh_init(void);
#endif
