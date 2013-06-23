#ifndef _LOGGER_H_
#define _LOGGER_H_

#include "psas_packet.h"
#include "gps.h"
#include "rollControlLibrary.h"

void logger_init(void); // [miml:init]
void logger_final(void); // [miml:final]

extern void log_getPositionData_adis(ADIS_packet *); // [miml:receiver]
void log_getPositionData_gps(GPS_packet*);
void log_getSignalData_arm(char*);
void log_getPositionData_rc(RollServo_adjustment*);
void log_getSignalData_rs(char*);

#endif
