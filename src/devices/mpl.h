/*
 * Interprets messages from the MPL311A52 pressure sensor
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

void MIML_SENDER mpl_data_out(MPLMessage*);
void MIML_RECEIVER mpl_raw_in(unsigned char*, unsigned int, unsigned char*);


#endif /* MPL_H_ */
