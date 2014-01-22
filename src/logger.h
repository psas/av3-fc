/*
 * Handles handles formatting and logging messages to disk and over WiFi
 */

#ifndef _LOGGER_H_
#define _LOGGER_H_

#include "adis.h"
#include "gps.h"
#include "mpl.h"
#include "mpu.h"
#include "rollcontrol.h"

void logger_init(void); // [miml:init]
void logger_final(void); // [miml:final]

extern void log_receive_adis(ADISMessage *); // [miml:receiver]
extern void log_receive_gps(GPSMessage*);
extern void log_receive_mpu(MPUMessage*);
extern void log_receive_mpl(MPLMessage*);
extern void log_receive_rc(RollServoMessage*);
extern void log_receive_arm(const char*);


#endif
