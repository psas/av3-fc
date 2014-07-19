/*
 * Interprets messages from the MPL311A52 pressure sensor
 */

#ifndef MPL_H_
#define MPL_H_

#include <stdint.h>

typedef struct {
	uint32_t pressure;
	int16_t temperature;
} __attribute__((packed)) MPLData;

typedef struct {
	char     ID[4];
	uint8_t  timestamp[6];
	uint16_t data_length;
	MPLData data;
} __attribute__((packed)) MPLMessage;

void mpl_data_out(MPLMessage*);
void mpl_raw_in(unsigned char*, unsigned int, unsigned char*);


#endif /* MPL_H_ */
