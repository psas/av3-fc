#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <stdbool.h>
#include "elderberry/fcfutils.h"
#include "utilities/net_addrs.h"
#include "utilities/utils_sockets.h"
#include "devices/gps.h"
#include "arm.h"

#define COMPARE_BUFFER_TO_CMD(a, b, len)\
	!strncmp((char*)a, b, sizeof(b) > len? len: sizeof(b))

static int sd;

bool slock_enable;
bool GPS_locked;

#define ACCEL_NOISE_BOUND 0.1
static int upright = 0;

int about(double a, double b){
	return fabs(a-b) < ACCEL_NOISE_BOUND;
}

void arm_receive_imu(const char *ID, uint8_t *timestamp, uint16_t len, void *buf){
	// does the acceleration vector == -1g over the last 100 samples?
	// 3.3mg per bit
	ADIS16405Data *data = buf;
	double x = data->acc_x * 0.00333;
	double y = data->acc_y * 0.00333;
	double z = data->acc_z * 0.00333;
	if(!about(x, -1) || !about(y, 0) || !about(z, 0)){
		upright = 0;
	}
	else if(upright < 100){
		++upright;
	}
}


/**
 * Receive data from the COTS GPS to determine fix status.
 * If we don't have GPS laock we shouldn't fly.
 */
void arm_receive_gps(V6NAMessage *fixdata) {

	// We must at least have a 3D fix
	if (fixdata->data.fix_mode >= VENUS6_FIX_MODE_3D) {
		GPS_locked = true;
	}
	else {
		GPS_locked = false;
	}
}


static void send_arm_response(const char * message){
	if(write(sd, message, strlen(message)) < 0){
		perror("send_arm_response: write() failed");
	}
}

void arm_raw_in(unsigned char *buffer, int unsigned len, unsigned char * timestamp){
	/* Commands:
	 * #YOLO - (You Only Launch Once) Arm the rocket for launch
	 * #SAFE - Disarm the rocket (default)
	 * EN_SLOCK - Enable sensors lock required to arm (default)
	 * DI_SLOCK - Disable sensors lock required to arm
	 */

	char ARM[] = "#YOLO";
	char ARM_response[] = "successful ARM";
	char ARM_decline_sensors[] = "ARM failed due to sensor lock";
	char SAFE[] ="#SAFE";
	char SAFE_response[] = "successful SAFE";
	char EN_SLOCK []= "EN_SLOCK";
	char EN_SLOCK_response[] = "successful EN_SLOCK";
	char DI_SLOCK []= "DI_SLOCK";
	char DI_SLOCK_response[] = "Sensor Lock Overridden";
	char UNKNOWN_COMMAND[] = "UNKNOWN COMMAND";

	if(COMPARE_BUFFER_TO_CMD(buffer, ARM, len)){
		//send arm
		bool accel_locked = upright == 100;
		bool sensors_allow_launch = (GPS_locked && accel_locked) || !slock_enable;

		if(sensors_allow_launch){
			arm_send_signal("ARM");
			send_arm_response(ARM_response);
		}
		else{
			send_arm_response(ARM_decline_sensors);
		}
	}
	else if(COMPARE_BUFFER_TO_CMD(buffer, SAFE, len)){
		//send safe
		arm_send_signal("SAFE");
		send_arm_response(SAFE_response);
	}
	else if(COMPARE_BUFFER_TO_CMD(buffer, EN_SLOCK, len)){
		//enable slock
		slock_enable = true;
		send_arm_response(EN_SLOCK_response);
	}
	else if(COMPARE_BUFFER_TO_CMD(buffer, DI_SLOCK, len)){
		//disable slock
		slock_enable = false;
		send_arm_response(DI_SLOCK_response);
	}
	else{
		//unknown command
		send_arm_response(UNKNOWN_COMMAND);
	}
}

void arm_init(void){
	slock_enable = true;
	sd = udp_socket();
	if(sd < 0){
		return;
	}
	if(connect(sd, ARM_ADDR, sizeof(struct sockaddr_in)) < 0){
		perror("arm_init: connect() failed");
		close(sd);
	}
}

void arm_final(void){
	close(sd);
}
