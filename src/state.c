#include <math.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include "elderberry/fcfutils.h"
#include "state.h"

static StateData current_state;
static double dt = 1/819.2;


void state_init(void) {
	current_state.time = 0;
	current_state.acc_up = 0;
	current_state.vel_up = 0;
	current_state.altitude = LAUNCH_ALTITUDE;
	current_state.roll_rate = 0;
	current_state.roll_angle = 0;
}


/**
 * Recieve data from the IMU, and integrate to a state estimation
 */
void state_receive_imu(ADISMessage *imu) {

	// Convert raw IMU data to MKS/degrees unit system
	const double accel = ADIS_GLSB * (int16_t) ntohs(imu->data.acc_x);
	const double roll_rate = ADIS_RLSB * (int16_t) ntohs(imu->data.gyro_x);

	// Integrate sensors
	current_state.acc_up = accel;
	current_state.vel_up += accel*dt;
	current_state.altitude += current_state.vel_up*dt;
	current_state.roll_rate = roll_rate;
	current_state.roll_angle += roll_rate*dt;

	// Send data
	VSTEMessage packet ={
		.ID={"VSTE"},
		.timestamp={(uint8_t)imu->timestamp[0], (uint8_t)imu->timestamp[1],
					(uint8_t)imu->timestamp[2], (uint8_t)imu->timestamp[3],
					(uint8_t)imu->timestamp[4], (uint8_t)imu->timestamp[5]},
		.data_length=sizeof(StateData),
	};
	memcpy(&packet.data, &current_state, sizeof(StateData));
	state_send_message(&packet);
}


void state_raw_ld_in(unsigned char *signal, unsigned int len, unsigned char *timestamp) {

}
