/*
 * arm.h
 *
 *  Created on: Jun 22, 2013
 *      Author: theo
 */

#ifndef ARM_H_
#define ARM_H_
#include "gps.h"

void arm_getSignalData_adis(ADIS_packet *);
void arm_getPositionData_gps(GPS_packet *);
#endif /* ARM_H_ */
