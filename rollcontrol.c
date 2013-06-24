#include <stdint.h>
#include "rollcontrol.h"
#include "rollControlLibrary.h"
#include "psas_packet.h"

uint8_t launch;

void rollcontrol_init(void){
	launch = 0;
	void rc_init();
}

void rollcontrol_final(void){
	return;
}

void rc_getPositionData_adis(ADIS_packet * imu){
	RC_INPUT_STRUCT_TYPE input;
	RC_OUTPUT_STRUCT_TYPE output;

	input.u16RawAccelerometerADC = imu->data.adis_zaccl_out;
	input.u16RawRateGyroADC = imu->data.adis_zgyro_out;
	input.u8IsLaunchDetected = launch;
	rc_step(&input, &output);
	output.u8ServoDisableFlag=0;
	output.u16ServoPulseWidthBin14 = imu->data.adis_zgyro_out;
	sendRollControlData(&output);
}

void rc_getSignalData_arm(char * signal){
	if(signal[0]){
		launch = 1;
	}else{
		launch = 0;
	}
}

void rc_getLaunchDetect(unsigned char * signal, int len, unsigned char* timestamp){
	launch = 1;
}
