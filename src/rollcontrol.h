/*
 * Handles the logic of turning sensor data into roll control fin position
 */

#ifndef ROLLCONTROL_H_
#define ROLLCONTROL_H_

#include "adis.h"

#define NULL_SERVO_POSITION 1500

typedef struct{
	char     ID[4];
	uint8_t  timestamp[6];
	uint16_t data_length;
	uint16_t u16ServoPulseWidthBin14;
	uint8_t u8ServoDisableFlag;
}__attribute__((packed)) RollServoMessage;

void rollcontrol_init(void); // [miml:init]

void rc_receive_imu(ADISMessage *); // [miml:receiver]
void rc_receive_arm(const char *); // [miml:receiver]
void rc_raw_ld_in(unsigned char *, unsigned int, unsigned char*); // [miml:receiver]
void rc_raw_testrc(unsigned char *, unsigned int, unsigned char*); // [miml:receiver]


void rc_send_servo(RollServoMessage*); // [miml:sender]

#endif /* ROLLCONTROL_H_ */

