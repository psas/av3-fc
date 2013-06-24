/*
 * mpl.h
 *
 */

#ifndef MPL_H_
#define MPL_H_

typedef struct {
	char     ID[4];
	uint8_t  timestamp[6];
	uint16_t data_length;
	unsigned char * data;
}__attribute__((packed)) MPL_packet;

extern void sendMPLData(MPL_packet*); // [miml:sender]

extern void mpl_getRawData_eth(unsigned char*, int, unsigned char*); // [miml:reciever]


#endif /* MPL_H_ */
