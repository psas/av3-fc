#ifndef _LOGGER_H_
#define _LOGGER_H_
void logger_init(void); // [miml:init]
void logger_final(void); // [miml:final]
extern void log_getPositionData_adis(ADIS_packet*); // [miml:receiver]
#endif
