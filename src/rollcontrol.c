#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/timerfd.h>
#include "elderberry/fcfutils.h"
#include "utilities/utils.h"
#include "utilities/utils_time.h"
#include "utilities/net_addrs.h"
#include "utilities/utils_sockets.h"
#include "rollcontrol.h"

static int sd;

static bool launch;
static bool enable_servo;
static bool armed;

static struct timespec last_time;
static double last_roll_error;
static double roll_error_integral;

static void set_servo_enable(bool enable)
{
	last_time = (struct timespec) { 0, 0 };
	last_roll_error = 0;
	roll_error_integral = 0;
	enable_servo = enable;
}

static void set_armed(bool arm)
{
	armed = arm;
	set_servo_enable(arm);
}

void rollcontrol_init(void){
	launch = false;
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

	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);

	/* ADIS fixed-to-float conversion */
	const double rate_deg = 0.05 * (int16_t) ntohs(imu->data.adis_gyro_x);

	const double Kp = 1;
	const double Ki = 0;
	const double Kd = 0;

	const double roll_error = rate_deg;
	double output = Kp * roll_error;

	if (last_time.tv_sec || last_time.tv_nsec) {
		double dt = now.tv_sec - last_time.tv_sec + now.tv_nsec / 1e9 - last_time.tv_nsec / 1e9;
		roll_error_integral += roll_error * dt;
		double roll_error_derivative = (roll_error - last_roll_error) / dt;
		output += Ki * roll_error_integral + Kd * roll_error_derivative;
	}

	set_canard_angle(output);

	last_time = now;
	last_roll_error = roll_error;
}

void rc_receive_arm(const char * signal){
	if(!strcmp(signal, "ARM")){
		set_armed(true);
		launch = true;
	}else if(!strcmp(signal, "SAFE")){
		set_armed(false);
		// TODO: Send final message
	}
}

void rc_raw_ld_in(unsigned char * signal, unsigned int len, unsigned char* timestamp){
	if(len > 0){
		launch = (signal[0]) ? 1 : 0;
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
