#ifndef _LOGGER_H_
#define _LOGGER_H_

//#include "psas_packet.h"
void logger_init(void); // [miml:init]
void logger_final(void); // [miml:final]
extern void log_getPositionData_adis(unsigned char *, int); // [miml:receiver]
//extern void log_getPositionData_adis(ADIS_packet*); 
#endif
