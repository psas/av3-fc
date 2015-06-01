/*
 * Handles the logic of turning sensor data into roll control fin position
 */

#ifndef ROLLCONTROL_H_
#define ROLLCONTROL_H_

#include "adis.h"

#define MAX_CANARD_ANGLE 15.0            // Canard position in Degrees
#define MIN_CANARD_ANGLE -15.0           // Canard position in Degrees

/* PID */
#define KP 10.0

/* Fin charachterization constants */
#define FINFIT_A 0.0006
#define FINFIT_B 0.045
#define FIN_AREA 1.13e-3
#define FIN_ARM 0.085

// Converts degrees to radians.
#define degreesToRadians(angleDegrees) (angleDegrees * M_PI / 180.0)
 
// Converts radians to degrees.
#define radiansToDegrees(angleRadians) (angleRadians * 180.0 / M_PI)


typedef struct{
	char     ID[4];
	uint8_t  timestamp[6];
	uint16_t data_length;
	double   finangle;
	uint8_t  servoDisableFlag;
} __attribute__((packed)) RollServoMessage;

void rollcontrol_init(void); // [miml:init]

void rc_receive_imu(ADISMessage *); // [miml:receiver]
void rc_receive_arm(const char *); // [miml:receiver]
void rc_raw_ld_in(unsigned char *, unsigned int, unsigned char*); // [miml:receiver]
void rc_raw_testrc(unsigned char *, unsigned int, unsigned char*); // [miml:receiver]


void rc_send_servo(RollServoMessage*); // [miml:sender]

#endif /* ROLLCONTROL_H_ */

