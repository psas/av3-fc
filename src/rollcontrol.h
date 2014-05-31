/*
 * Handles the logic of turning sensor data into roll control fin position
 */

#ifndef ROLLCONTROL_H_
#define ROLLCONTROL_H_

#include <ev.h>
#include "adis.h"

#define NULL_SERVO_POSITION 1500

/// TODO: FIX
#define MAX_SERVO_POSITION 3000.0 // PWM
#define MIN_SERVO_POSITION 0.0    // PWM
#define MAX_GRATE 50.0            // Degrees/s
#define MIN_GRATE -50.0           // Degrees/s

typedef struct{
	char     ID[4];
	uint8_t  timestamp[6];
	uint16_t data_length;
	uint16_t u16ServoPulseWidthBin14;
	uint8_t u8ServoDisableFlag;
}__attribute__((packed)) RollServoMessage;

void MIML_INIT rollcontrol_init(struct ev_loop * loop);
void MIML_RECEIVER rc_receive_imu(ADISMessage *);
void MIML_RECEIVER rc_receive_arm(const char *);
void MIML_RECEIVER rc_raw_ld_in(unsigned char *, unsigned int, unsigned char*);
void MIML_RECEIVER rc_raw_testrc(unsigned char *, unsigned int, unsigned char*);
void MIML_SENDER rc_send_servo(RollServoMessage*);

#endif /* ROLLCONTROL_H_ */

