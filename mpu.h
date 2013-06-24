/*
 * mpu.h
 *
 */

#ifndef MPU_H_
#define MPU_H_


typedef struct {
	char     ID[4];
	uint8_t  timestamp[6];
	uint16_t data_length;
	unsigned char * data;
}__attribute__((packed)) MPU_packet;

extern void sendMPUData(MPU_packet*); // [miml:sender]

extern void mpu_getRawData_eth(unsigned char*, int, unsigned char*); // [miml:reciever]




#endif /* MPU_H_ */
