#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/timerfd.h>
#include "elderberry/fcfutils.h"
#include "utilities/utils_time.h"
#include "utilities/net_addrs.h"
#include "utilities/utils_sockets.h"
#include "rollcontrol.h"

int sd;

static bool launch;
static bool enable_servo;
static bool armed;

void rollcontrol_init(void){
	launch = false;
	enable_servo = false;

	sd = udp_socket();
	if(sd < 0){
		return;
	}
	if(connect(sd, RC_SERVO_ENABLE_ADDR, sizeof(struct sockaddr_in)) < 0){
		perror("rollcontrol_init: connect() failed");
		close(sd);
	}

}

void rc_receive_imu(ADISMessage * imu){

	if (!enable_servo)
		return;

	int16_t rate = imu->data.adis_gyro_x;
	double rate_deg = 0.05 *  rate;

	// Slope
	double a = (MAX_SERVO_POSITION - MIN_SERVO_POSITION) / (MAX_GRATE - MIN_GRATE);
	// Inercept
	double b = MIN_SERVO_POSITION - (a * MIN_GRATE);

	double servo = a*rate_deg + b;
	if (servo > MAX_SERVO_POSITION)
		servo = MAX_SERVO_POSITION;
	if (servo < MIN_SERVO_POSITION)
		servo = MIN_SERVO_POSITION;

	RollServoMessage out = {
			.ID = {"ROLL"},
			.data_length = 3,
			.u16ServoPulseWidthBin14 = servo,
			.u8ServoDisableFlag = !enable_servo,
	};
	get_psas_time(out.timestamp);

	rc_send_servo(&out);
}

void rc_receive_arm(const char * signal){
	if(!strcmp(signal, "ARM")){
		armed = true;
		enable_servo = true;
		launch = true;
	}else if(!strcmp(signal, "SAFE")){
		armed = false;
		enable_servo = false;
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
