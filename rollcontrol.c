#include <stdbool.h>
#include "rollcontrol.h"
#include "rollControlLibrary.h"
#include <stdio.h>
static bool launch;

void rollcontrol_init(void){
	launch = 0;
	rc_init();
}

uint16_t scale_accel(int16_t acc){
	const double shift = 34500 + 1374.05;
	const double scale = 4.57559; // (0.003 g/adis_lsb) * (1374.05 rc_counts/g)
	return (uint16_t)(((double)-acc)*scale + shift);
}

uint16_t scale_gyro(int16_t gyr){
	const double shift = 29400;
	const double scale = 2.1845;// (0.05 deg/sec/adislsb) * (43.69 rc_counts/deg/sec)
	return (uint16_t)(((double)-gyr)*scale + shift);
}
void rc_receive_imu(ADISMessage * imu){
	RC_INPUT_STRUCT_TYPE input;
	RC_OUTPUT_STRUCT_TYPE output;

	input.u16RawAccelerometerADC = scale_accel(imu->data.adis_xaccl_out);
	input.u16RawRateGyroADC = scale_gyro(imu->data.adis_xgyro_out);
	input.u8IsLaunchDetected = launch;
	rc_step(&input, &output);
	output.u8ServoDisableFlag=0;

	RollServoMessage out = {
			.ID = {"ROLL"},
			.timestamp = {0,0,0,0,0,0},
			.data_length = 3,
			.u16ServoPulseWidthBin14 = output.u16ServoPulseWidthBin14,
			.u8ServoDisableFlag = output.u8ServoDisableFlag,
	};

	rc_send_servo(&out);
}

void rc_receive_arm(char * signal){
	if(signal[0]){
		launch = 1;
	}else{
		launch = 0;
	}
}

void rc_raw_ld_in(unsigned char * signal, int len, unsigned char* timestamp){
	launch = (signal[0]) ? 1 : 0;
}
