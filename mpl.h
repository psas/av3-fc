/*
 * mpl.h
 *
 */

#ifndef MPL_H_
#define MPL_H_

#include <stdint.h>

typedef struct {
	char     ID[4];
	uint8_t  timestamp[6];
	uint16_t data_length;
	unsigned char * data;
}__attribute__((packed)) MPLMessage;

extern void mpl_data_out(MPLMessage*); // [miml:sender]

extern void mpl_raw_in(unsigned char*, unsigned int, unsigned char*); // [miml:reciever]


#endif /* MPL_H_ */
