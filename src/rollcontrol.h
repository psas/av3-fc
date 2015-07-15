/*
 * Handles the logic of turning sensor data into roll control fin position
 */

#ifndef ROLLCONTROL_H_
#define ROLLCONTROL_H_

#include "utilities/psas_packet.h"

#define MAX_CANARD_ANGLE 15.0            // Canard position in Degrees
#define MIN_CANARD_ANGLE -15.0           // Canard position in Degrees

/* PID gain constants */
#define Kp 50
#define Ki .009
#define Kd 0
#define pidTarget 0


/* Fin charachterization constants */
#define FINFIT_A 0.0006
#define FINFIT_B 0.045
#define FIN_CBASE 3.2
#define FIN_AREA 1.13e-3
#define FIN_ARM 0.085
#define I_INIT 0.086
#define I_BO 0.077
#define EXPECTED_BURN_TIME 5.7
#define SUBSONIC 265
#define SUPERSONIC 330

// Converts degrees to radians.
#define degreesToRadians(angleDegrees) (angleDegrees * M_PI / 180.0)
 
// Converts radians to degrees.
#define radiansToDegrees(angleRadians) (angleRadians * 180.0 / M_PI)

/* global variables needed for PID controller */
double lastError;
double integrator;

void rollcontrol_init(void); // [miml:init]

void rc_receive_state(const char *ID, uint8_t *timestamp, uint16_t len, void *buf); // [miml:receiver]
void rc_receive_arm(const char *); // [miml:receiver]
void rc_raw_umb(const char *, unsigned char*, unsigned int, void*); // [miml:receiver]
void rc_raw_testrc(unsigned char *, unsigned int, unsigned char*); // [miml:receiver]


void rc_send_servo(ROLLMessage*); // [miml:sender]

#endif /* ROLLCONTROL_H_ */

