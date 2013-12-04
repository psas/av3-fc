#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/timerfd.h>
#include "utils_time.h"
#include "elderberry/fcfutils.h"
#include "net_addrs.h"
#include "utils_sockets.h"
#include "rollcontrol.h"

int sd;

static bool launch;
static bool enable_servo;
static bool armed;
static uint16_t accel;
static uint16_t roll;
static bool prev_servo_disable;

static uint16_t scale_accel(int16_t acc){
	const double shift = 34500 + 1374.05;
	const double scale = 4.57559; // (0.003 g/adis_lsb) * (1374.05 rc_counts/g)
	return (uint16_t)(((double)-acc)*scale + shift);
}

static uint16_t scale_gyro(int16_t gyr){
	const double shift = 29400;
	const double scale = 2.1845;// (0.05 deg/sec/adislsb) * (43.69 rc_counts/deg/sec)
	return (uint16_t)(((double)-gyr)*scale + shift);
}

static void step(struct pollfd * pfd){
	char buf[8];
	read(pfd->fd, buf, 8); //clears timerfd

	// Do control algorithm here

	RollServoMessage out = {
			.ID = {"ROLL"},
			.data_length = 3,
			.u16ServoPulseWidthBin14 = NULL_SERVO_POSITION,
			.u8ServoDisableFlag = !enable_servo,
	};

	get_psas_time(out.timestamp);

	// Servo disable flag gets sent, and then stop sending packets
	if(!prev_servo_disable || !out.u8ServoDisableFlag){
		rc_send_servo(&out);
		prev_servo_disable = out.u8ServoDisableFlag;
	}
}

void rollcontrol_init(void){
	launch = false;
	enable_servo = false;
	prev_servo_disable = false;
	accel = scale_accel(0);
	roll = scale_gyro(0);

	int tfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
	struct itimerspec  newval;
	newval.it_interval.tv_sec = 0;
	newval.it_interval.tv_nsec = 1000000; //1 ms
	newval.it_value.tv_sec = 0;
	newval.it_value.tv_nsec = 1000000;
	timerfd_settime(tfd, 0, &newval, NULL);
	fcf_add_fd(tfd, POLLIN, step);
	sd = get_send_socket();
}

void rc_receive_imu(ADISMessage * imu){
	accel = scale_accel(imu->data.adis_xaccl_out);
	roll = scale_gyro(imu->data.adis_xgyro_out);
}

void rc_receive_arm(const char * signal){
	if(!strcmp(signal, "ARM")){
		armed = true;
		enable_servo = true;
		launch = true;
	}else if(!strcmp(signal, "SAFE")){
		armed = false;
		enable_servo = false;
	}
}

void rc_raw_ld_in(unsigned char * signal, unsigned int len, unsigned char* timestamp){
	if(len > 0){
		launch = (signal[0]) ? 1 : 0;
	}
}

static void send_servo_response(const char * message){
	sendto_socket(sd, message, strlen(message), ARM_IP, RC_SERVO_ENABLE_PORT);
}

#define COMPARE_BUFFER_TO_CMD(a, b, len)\
	!strncmp((char*)a, b, sizeof(b) > len? len: sizeof(b))


void rc_raw_testrc(unsigned char * data, unsigned int len, unsigned char* timestamp){
	if(!armed){
		if(COMPARE_BUFFER_TO_CMD(data, "ENABLE", len)){
			enable_servo = true;
			send_servo_response("Roll control servos enabled");
		}
		else if(COMPARE_BUFFER_TO_CMD(data, "DISABLE", len)){
			enable_servo = false;
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
