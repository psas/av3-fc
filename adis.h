/*
 * virtdevsrv.h
 *
 *  Created on: Jan 26, 2013
 */

#ifndef VIRTDEVSRV_H_
#define VIRTDEVSRV_H_

#include "psas_packet.h"
extern int adis_init(void); // [miml:init]
extern void adis_final(void); // [miml:final]
extern void sendADISData(ADIS_packet *data); // [miml:sender]

#endif /* VIRTDEVSRV_H_ */
