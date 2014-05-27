/*
 * Interprets data from the MPU9150 IMU
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

void MIML_SENDER mpu_data_out(MPUMessage*);
void MIML_RECEIVER mpu_raw_in(unsigned char*, unsigned int, unsigned char*);

#endif /* MPU_H_ */
