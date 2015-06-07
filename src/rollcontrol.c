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
#include "rollcontrol.h"

static int sd;

static bool enable_servo;
static bool armed;

static double time_since_launch;
static double velocity;
static double altitude;

static void set_servo_enable(bool enable)
{
	time_since_launch = 0;
	velocity = 0;
	altitude = LAUNCH_ALTITUDE;
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
	RollServoMessage out = {
			.ID = {"ROLL"},
			.data_length = 9,
			.finangle = degrees,
			.servoDisableFlag = !enable_servo,
	};
	get_psas_time(out.timestamp);

	rc_send_servo(&out);
}


/**
 * Subsonic fin estimation
 */
double subsonic_fin(double set_aa, double I, double rd);
double subsonic_fin(double set_aa_rad, double I, double rd) {
	double alpha = sqrt(fabs(2*set_aa_rad*I*FINFIT_A)/(rd*velocity*velocity*FIN_AREA*FIN_ARM) + FINFIT_B*FINFIT_B) - FINFIT_B;
	alpha = alpha / (2*FINFIT_A);
	return alpha;
}

/**
 * Supersonic fin estimation
 */
double supersonic_fin(double set_aa, double I, double rd);
double supersonic_fin(double set_aa, double I, double rd) {
	double alpha = (set_aa*I)/(2*rd*velocity*velocity*FIN_AREA*FIN_ARM*FIN_CBASE);
	return radiansToDegrees(alpha);
}


/**
 * Given a correction from control system, estimate correct angle of attack for canard
 */
double estimate_alpha(double set_aa);
double estimate_alpha(double set_aa) {

	// obvious cases (and avoid divide by 0):
	if ((fabs(set_aa) < 1)  || (velocity < 1))
		return 0;

	double aa = fabs(degreesToRadians(set_aa));

	// Compute inertia
	double I = I_INIT;
	if (time_since_launch < EXPECTED_BURN_TIME) {
		I = I_INIT + (I_BO - I_INIT)*(time_since_launch / EXPECTED_BURN_TIME);
	} else {
		I = I_BO;
	}

	// Compute atmosphere
	double rd = 1.2250 * exp((-9.80665 * 0.0289644 * altitude)/(8.31432*288.15)); 


	// default is 0
	double output = 0;

	// Subsonic, transonic, and supersonic cases:
	if (velocity <= SUBSONIC)
		output = subsonic_fin(aa, I, rd);
	else if (velocity < SUPERSONIC) {	
		double y0 = subsonic_fin(aa, I, rd);
        double y1 = supersonic_fin(aa, I, rd);
        output = y0 + (y1-y0)*(velocity - SUBSONIC)/(SUPERSONIC-SUBSONIC);
	}
	else
		output = supersonic_fin(aa, I, rd);

	// Make negative if nessisary
    if (set_aa < 0)
        return -output;
    return output;
}

void rc_receive_imu(ADISMessage * imu){

	if (!enable_servo)
		return;

	/* ADIS fixed-to-float conversion to m/s/s */
	const double accel = (0.00333 * 9.80665) * (int16_t) ntohs(imu->data.adis_acc_x);

	/* don't start integrating until it looks like we've launched */
	if (armed && (time_since_launch <= 0 && fabs(accel - 9.80665) < 20 ))
		return;

	const double dt = 1 / 819.2;
	time_since_launch += dt;
	velocity += (accel - 9.80665) * dt;
    altitude += velocity * dt;

	/* Proportional Controller */
	// Roll rate:
	const double roll_rate = 0.05 * (int16_t) ntohs(imu->data.adis_gyro_x);


	// Error amd PID Constants
	double error = 0 - roll_rate;
	double proportional = KP * error;
	double correction = proportional;  // TODO: expand to PI or PID

	// Look normilized fin angle based on correctio
	double output = estimate_alpha(correction);

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

void rc_raw_ld_in(unsigned char * signal, unsigned int len, unsigned char* timestamp){
	/* we're ignoring launch detect for this launch */
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
