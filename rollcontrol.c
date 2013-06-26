#include <stdbool.h>
#include "rollcontrol.h"
#include "rollControlLibrary.h"

static bool launch;

void rollcontrol_init(void){
	launch = 0;
	rc_init();
}

void rc_receive_imu(ADISMessage * imu){
	RC_INPUT_STRUCT_TYPE input;
	RC_OUTPUT_STRUCT_TYPE output;

	input.u16RawAccelerometerADC = imu->data.adis_zaccl_out;
	input.u16RawRateGyroADC = imu->data.adis_zgyro_out;
	input.u8IsLaunchDetected = !launch;
	rc_step(&input, &output);
	output.u8ServoDisableFlag=0;
	output.u16ServoPulseWidthBin14 = imu->data.adis_zgyro_out;

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
	launch = 1;
}
