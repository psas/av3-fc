/*
 * Handles the logic of turning sensor data into roll control fin position
 */

#ifndef ROLLCONTROL_H_
#define ROLLCONTROL_H_

#include "adis.h"

/// TODO: FIX
#define MAX_SERVO_POSITION 3000.0 // PWM
#define MIN_SERVO_POSITION 0.0    // PWM
#define MAX_CANARD_ANGLE 50.0            // Canard position in Degrees
#define MIN_CANARD_ANGLE -50.0           // Canard position in Degrees

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

