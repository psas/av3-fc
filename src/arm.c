#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <stdbool.h>
#include "elderberry/fcfutils.h"
#include "utilities/net_addrs.h"
#include "utilities/utils_sockets.h"
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

void arm_receive_imu(ADISMessage * data){
	// does the acceleration vector == -1g over the last 100 samples?
	// 3.3mg per bit
	double x = data->data.acc_x * 0.00333;
	double y = data->data.acc_y * 0.00333;
	double z = data->data.acc_z * 0.00333;
	if(!about(x, -1) || !about(y, 0) || !about(z, 0)){
		upright = 0;
	}
	else if(upright < 100){
		++upright;
	}
}

void arm_receive_gps(GPSMessage * d){
	if (memcmp(d->ID, "GPS\x01", 4))
		return;

	GPS1Message * data = (GPS1Message *)d;
	switch(data->data.nav_mode) {
	case 2:   // 3D fix
	case 4:   // 3D + diff
	case 6:   // 3D + diff + rtk
		GPS_locked = true; break;
	default:
		GPS_locked = false; break;
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
