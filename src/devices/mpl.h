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

#endif /* MPL_H_ */
