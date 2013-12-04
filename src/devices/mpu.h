/*
 * mpu.h
 *
 */

#ifndef MPU_H_
#define MPU_H_

#include <stdint.h>

typedef struct {
	char     ID[4];
	uint8_t  timestamp[6];
	uint16_t data_length;
	unsigned char * data;
}__attribute__((packed)) MPUMessage;

extern void mpu_data_out(MPUMessage*); // [miml:sender]

extern void mpu_raw_in(unsigned char*, unsigned int, unsigned char*); // [miml:reciever]

#endif /* MPU_H_ */
