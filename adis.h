/*
 * adis.h
 *
 */

#ifndef ADIS_H_
#define ADIS_H_
#include "psas_packet.h"

extern void sendADISData(ADIS_packet*); // [miml:sender]

extern void adis_getRawData_eth(unsigned char*, int, unsigned char*); // [miml:reciever]

#endif /* ADIS_H_ */
