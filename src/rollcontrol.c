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

static void set_servo_enable(bool enable)
{
	time_since_launch = 0;
	velocity = 0;
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

	/*
	 * f(v) = SWEEP_COEFFICIENT / v**2 + SWEEP_MIN
	 *
	 * Desired limits:
	 * f(40 m/s) = 15 degrees # sweep at slow speeds
	 * f(338 m/s) = 0.5 degrees # sweep at predicted peak velocity
	 *
	 * The following values were fitted to the above constraints.
	 */
	const double SWEEP_COEFFICIENT = 23529.5;
	const double SWEEP_MIN = 0.294041;

	/* for these purposes, we'll pretend speeds below 40 m/s don't exist */
	const double sweep_vel = MAX(40, fabs(velocity));
	const double sweep_amplitude = SWEEP_COEFFICIENT / (sweep_vel * sweep_vel) + SWEEP_MIN;

	/* sweep the canards through a sweep_amplitude-modulated triangle wave */
	const double CYCLES_PER_SECOND = 1;
	const double half_cycles = time_since_launch * CYCLES_PER_SECOND * 2 + 0.5;
	const int integer_half_cycles = floor(half_cycles);
	const double fractional_half_cycles = half_cycles - integer_half_cycles;
	const int reverse = integer_half_cycles & 1 ? -1 : 1; /* odd-numbered half-cycles go the opposite direction */
	const double sweep_position = fractional_half_cycles * 2 - 1; /* transform [0,1] to [-1,1] */

	const double output = sweep_amplitude * sweep_position * reverse;

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
