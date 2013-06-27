#ifndef ROLLCONTROL_H_
#define ROLLCONTROL_H_

#include "adis.h"

typedef struct{
	char     ID[4];
	uint8_t  timestamp[6];
	uint16_t data_length;
	uint16_t u16ServoPulseWidthBin14;
	uint8_t u8ServoDisableFlag;
}__attribute__((packed)) RollServoMessage;

void rollcontrol_init(void); // [miml:init]

void rc_receive_imu(ADISMessage *); // [miml:receiver]
void rc_receive_arm(char *); // [miml:receiver]
void rc_raw_ld_in(unsigned char *, int, unsigned char*); // [miml:receiver]
void rc_raw_testrc(unsigned char *, int, unsigned char*); // [miml:receiver]


void rc_send_servo(RollServoMessage*); // [miml:sender]

#endif /* ROLLCONTROL_H_ */

