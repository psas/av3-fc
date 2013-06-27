#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/timerfd.h>
#include "utils_time.h"
#include "fcfutils.h"
#include "rollcontrol.h"
#include "rollControlLibrary.h"


static bool launch;
static bool enable_servo;
static uint16_t accel;
static uint16_t roll;

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
	RC_INPUT_STRUCT_TYPE input;
	RC_OUTPUT_STRUCT_TYPE output;

	input.u16RawAccelerometerADC = accel;
	input.u16RawRateGyroADC = roll;
	input.u8IsLaunchDetected = launch;

	rc_step(&input, &output);

	RollServoMessage out = {
			.ID = {"ROLL"},
			.data_length = 3,
			.u16ServoPulseWidthBin14 = output.u16ServoPulseWidthBin14,
			.u8ServoDisableFlag = output.u8ServoDisableFlag && !enable_servo,
	};
	get_psas_time(out.timestamp);
	rc_send_servo(&out);
}

void rollcontrol_init(void){
	launch = false;
	enable_servo = false;
	accel = scale_accel(0);
	roll = scale_gyro(0);
	rc_init();
	int tfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
	struct itimerspec  newval;
	newval.it_interval.tv_sec = 0;
	newval.it_interval.tv_nsec = 1000000; //1 ms
	newval.it_value.tv_sec = 0;
	newval.it_value.tv_nsec = 1000000;
	timerfd_settime(tfd, 0, &newval, NULL);
	fcf_add_fd(tfd, POLLIN, step);
}

void rc_receive_imu(ADISMessage * imu){
	accel = scale_accel(imu->data.adis_xaccl_out);
	roll = scale_gyro(imu->data.adis_xgyro_out);
}

void rc_receive_arm(char * signal){
	if(strcmp(signal, "ARM")){
		enable_servo = true;
	}else if(strcmp(signal, "SAFE")){
		enable_servo = false;
	}
}

void rc_raw_ld_in(unsigned char * signal, int len, unsigned char* timestamp){
	if(len > 0){
		launch = (signal[0]) ? 1 : 0;
	}
}

void rc_raw_testrc(unsigned char * signal, int len, unsigned char* timestamp){

}
