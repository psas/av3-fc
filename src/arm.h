/*
 * ARM = ARM Rocket Manager
 * Manages global rocket state
 */

#ifndef ARM_H_
#define ARM_H_
#include "gps.h"
#include "adis.h"

void MIML_INIT arm_init(void);
void MIML_FINAL arm_final(void);

void MIML_RECEIVER arm_receive_imu(ADISMessage*);
void MIML_RECEIVER arm_receive_gps(GPSMessage*);
void MIML_RECEIVER arm_raw_in(unsigned char *, unsigned int, unsigned char *);

void MIML_SENDER arm_send_signal(const char *);
#endif /* ARM_H_ */
