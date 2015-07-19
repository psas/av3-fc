#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include "state.h"
#include "elderberry/fcfutils.h"
#include "utilities/utils_time.h"

static StateData current_state;
static double dt = 1/819.2;
static bool has_launched;
static uint64_t launch_time;


void state_init(void) {
	has_launched = false;
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
void state_receive_imu(const char *ID, uint8_t *timestamp, uint16_t len, void *buf) {
	ADIS16405Data *imu = buf;

	// Convert raw IMU data to MKS/degrees unit system
	const double accel = ADIS_GLSB * (int16_t) ntohs(imu->acc_x);
	const double roll_rate = ADIS_RLSB * (int16_t) ntohs(imu->gyro_x);

	if (!has_launched && fabs(accel) > 40) {
		has_launched = true;
		launch_time = from_psas_time(timestamp);
	}

	/* Unconditionally forward raw measurements because they're just
	 * as accurate at any phase of the launch. */
	current_state.acc_up = accel;
	current_state.roll_rate = roll_rate;

	if (has_launched) {
		// Integrate sensors
		current_state.time = (from_psas_time(timestamp) - launch_time) / 1.0e9;
		current_state.vel_up += accel*dt;
		current_state.altitude += current_state.vel_up*dt;
		current_state.roll_angle += roll_rate*dt;
	}

	// Send data
	state_send_message("VSTE", timestamp, sizeof(StateData), &current_state);
}


void state_raw_ld_in(unsigned char *signal, unsigned int len, unsigned char *timestamp) {

}
