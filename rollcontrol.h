#ifndef ROLLCONTROL_H_
#define ROLLCONTROL_H_

#include "psas_packet.h"
#include "rollControlLibrary.h"

void rollcontrol_init(void); // [miml:init]
void rollcontrol_final(void); // [miml:final]
//void sendRollControlData(RollServo_adjustment *); // [miml:sender]
void rc_getPositionData_adis(ADIS_packet *); // [miml:receiver]
void rc_getSignalData_arm(char *); // [miml:receiver]
void rc_getSignalData_rs(char *); // [miml:receiver]
void sendRollControlData(RollServo_adjustment*);

#endif /* ROLLCONTROL_H_ */

