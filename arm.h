/*
 * arm.h
 *
 * ARM = ARM Rocket Manager
 */

#ifndef ARM_H_
#define ARM_H_
#include "gps.h"
#include "adis.h"

void arm_init(void);
void arm_final(void);

void arm_receive_imu(ADISMessage*);
void arm_receive_gps(GPSMessage*);
void arm_raw_in(unsigned char *, int, unsigned char *);

void arm_send_signal(char *);
#endif /* ARM_H_ */
