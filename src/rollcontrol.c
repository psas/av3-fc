#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include "elderberry/fcfutils.h"
#include "utilities/utils.h"
#include "utilities/utils_time.h"
#include "utilities/net_addrs.h"
#include "utilities/utils_sockets.h"
#include "devices/rnh.h"
#include "rollcontrol.h"

/* Important Stuff for PID loop! */

/* PID gain constants */
#define Kp 5.0
#define Ki 0.01
#define Kd 0

/* PID target */
#define pidTarget 0

/* for clamping integrator term in PID */
#define integrator_max (100 / Ki)
#define integrator_min (-100 / Ki)

/* global variables needed for PID controller */
static double lastError;
static double integrator;

/* End Important Stuff for PID loop! */


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

static int sd;

static bool enable_servo;
static bool armed;

#define TIMEOUT (120 * UINT64_C(1000000000))	// two minutes in nanoseconds

static bool launched = false;
static uint64_t timeout;

static void set_servo_enable(bool enable)
{
	enable_servo = enable;
}

static void set_armed(bool arm)
{
	armed = arm;
	set_servo_enable(arm);
}

void rollcontrol_init(void){
	set_armed(false);

	sd = udp_socket();
	if(sd < 0){
		return;
	}
	if(connect(sd, RC_SERVO_ENABLE_ADDR, sizeof(struct sockaddr_in)) < 0){
		perror("rollcontrol_init: connect() failed");
		close(sd);
	}

}

static void set_canard_angle(double degrees)
{
	degrees = CLAMP(degrees, MIN_CANARD_ANGLE, MAX_CANARD_ANGLE);
	ROLLMessage out = {
		.ID = {"ROLL"},
		.data_length = sizeof(RollServoData),
		.data = {
			.angle = degrees,
			.disable = !enable_servo,
		},
	};
	get_psas_time(out.timestamp);

	rc_send_servo(&out);
}


/**
 * Subsonic fin estimation
 */
static double subsonic_fin(double set_aa_rad, double I, double rd, StateData state) {
	double v = state.vel_up;
	double alpha = sqrt(fabs(2*set_aa_rad*I*FINFIT_A)/(rd*v*v*FIN_AREA*FIN_ARM) + FINFIT_B*FINFIT_B) - FINFIT_B;
	alpha = alpha / (2*FINFIT_A);
	return alpha; // Orginial fit variables include degress/radians, so this return is in degrees
}

/**
 * Supersonic fin estimation
 */
static double supersonic_fin(double set_aa_rad, double I, double rd, StateData state) {
	double v = state.vel_up;
	double alpha = (set_aa_rad*I)/(2*rd*v*v*FIN_AREA*FIN_ARM*FIN_CBASE);
	return radiansToDegrees(alpha);
}


/**
 * Given a correction from control system, estimate correct angle of attack for canard
 */
static double estimate_alpha(double set_aa, StateData state) {

	double velocity = state.vel_up;
	double time = state.time;

	// obvious cases (and avoid divide by 0):
	if ((fabs(set_aa) < 1)  || (velocity < 1))
		return 0;

	double aa = fabs(degreesToRadians(set_aa));

	// Compute inertia
	double I = I_INIT;
	if (time < EXPECTED_BURN_TIME) {
		I = I_INIT + (I_BO - I_INIT)*(time / EXPECTED_BURN_TIME);
	} else {
		I = I_BO;
	}

	// Compute atmosphere
	double rd = 1.2250 * exp((-9.80665 * 0.0289644 * state.altitude)/(8.31432*288.15)); 


	// default is 0
	double output = 0;

	// Subsonic, transonic, and supersonic cases:
	if (velocity <= SUBSONIC)
		output = subsonic_fin(aa, I, rd, state);
	else if (velocity < SUPERSONIC) {	
		double y0 = subsonic_fin(aa, I, rd, state);
		double y1 = supersonic_fin(aa, I, rd, state);
		output = y0 + (y1-y0)*(velocity - SUBSONIC)/(SUPERSONIC-SUBSONIC);
	}
	else
		output = supersonic_fin(aa, I, rd, state);

    // Make negative if nessisary
    if (set_aa < 0)
        return -output;
    return output;
}

// rnhumb message, assume disconnect is launch detect
void rc_raw_umb(const char *ID, unsigned char* timestamp, unsigned int len, void* data)
{
	if (memcmp(ID, "RNHU", 4))
		return;

	RNHUmbdet *umb = (RNHUmbdet *)data;
	if (!launched && !umb->detect)		// did umbilical just now disconnect?
		timeout = from_psas_time(timestamp) + TIMEOUT;

	launched = !umb->detect;
}

static void check_timeout(const uint8_t* timestamp)
{
	if (!launched || !enable_servo) return;

	if (from_psas_time(timestamp) > timeout)
	{
		// timeout: disable the canards
		// NOTE: won't center them, disabling has precedence
		// TODO: make a little state machine to center, wait, then disable
		set_servo_enable(false);
		set_canard_angle(0);
	}
}

void rc_receive_state(const char *ID, uint8_t *timestamp, uint16_t len, void *buf) {

	check_timeout(timestamp);

	if (!enable_servo)
		return;

	if (memcmp(ID, "VSTE", 4))
		return;

	StateData *state = buf;

	/* begin PID Controller */

	/* 
	   determine the error by taking the difference of the target
	   and the current value
	 */
	double error = pidTarget - state->roll_rate;
	
	/* proportional stage */
	double proportional = Kp * error;

	/* integral stage */
	double integral = Ki * integrator;

	/* derivative stage */
	double derivative = Kd * (error - lastError);

	/* output of the PID controller */
	/* sum each stage together */
	double correction = proportional + integral + derivative;

	/* remember the error for the derivative stage */
	lastError = error;

	/* add the error to the integral stage for next step */
	integrator += error;

	/* clamp the integrator! */
	if(integrator > integrator_max){
	  integrator = integrator_max;
	}
	else if(integrator < integrator_min){
	  integrator = integrator_min;
	}

	// Look normilized fin angle based on requested angular acceleration
	double output = estimate_alpha(correction, *state);

	/* end PID controller */

	set_canard_angle(output);
}

void rc_receive_arm(const char * signal){
	if(!strcmp(signal, "ARM")){
		set_armed(true);
	}else if(!strcmp(signal, "SAFE")){
		set_armed(false);
		set_canard_angle(0);
	}
}

static void send_servo_response(const char * message){
	int len = strlen(message);
	if(write(sd, message, len) != len){
		perror("send_servo_response: write failed");
	}
}


#define COMPARE_BUFFER_TO_CMD(a, b, len)\
	!strncmp((char*)a, b, sizeof(b) > len? len: sizeof(b))


void rc_raw_testrc(unsigned char * data, unsigned int len, unsigned char* timestamp){
	if(!armed){
		if(COMPARE_BUFFER_TO_CMD(data, "ENABLE", len)){
			set_servo_enable(true);
			send_servo_response("Roll control servos enabled");
		}
		else if(COMPARE_BUFFER_TO_CMD(data, "DISABLE", len)){
			set_servo_enable(false);
			set_canard_angle(0);
			send_servo_response("Roll control servos disabled");
		}
		else{
			send_servo_response("Unknown servo command");
		}
	}
	else{
		send_servo_response("Roll control servos state not changed due to being armed");
	}
}
