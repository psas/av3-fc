/*
 * virtdevsrv.h
 *
 *  Created on: Jan 26, 2013
 */

#ifndef VIRTDEVSRV_H_
#define VIRTDEVSRV_H_
#include "psas_packet.h"
extern void adis_init(void); // [miml:init]
extern void adis_final(void); // [miml:final]
extern void sendADISData(ADIS_packet *); // [miml:sender]
extern void adis_getRawData_eth(char *buffer); // [miml:reciever]
#endif /* VIRTDEVSRV_H_ */
