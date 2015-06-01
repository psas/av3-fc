#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include "elderberry/fcfutils.h"
#include "utilities/utils.h"
#include "utilities/utils_time.h"
#include "utilities/net_addrs.h"
#include "utilities/utils_sockets.h"
#include "rollcontrol.h"

static int sd;

static bool enable_servo;
static bool armed;

static double time_since_launch;
static double velocity;

static void set_servo_enable(bool enable)
{
	time_since_launch = 0;
	velocity = 0;
	enable_servo = enable;
}

static void set_armed(bool arm)
{
	armed = arm;
	set_servo_enable(arm);
}

void rollcontrol_init(void){
	set_armed(false);

	sd = udp_socket();
	if(sd < 0){
		return;
	}
	if(connect(sd, RC_SERVO_ENABLE_ADDR, sizeof(struct sockaddr_in)) < 0){
		perror("rollcontrol_init: connect() failed");
		close(sd);
	}

}

static void set_canard_angle(double degrees)
{
	degrees = CLAMP(degrees, MIN_CANARD_ANGLE, MAX_CANARD_ANGLE);
	RollServoMessage out = {
			.ID = {"ROLL"},
			.data_length = 9,
			.finangle = degrees,
			.servoDisableFlag = !enable_servo,
	};
	get_psas_time(out.timestamp);

	rc_send_servo(&out);
}

/**
 * Given a correction from control system, estimate correct angle of attack for canard
 */
double estimate_alpha(double set_aa, double x, double v, double t);
double estimate_alpha(double set_aa, double x, double v, double t) {

	// obvious cases (and avoid divide by 0):
	if ((fabs(set_aa) < 1)  || (v < 1))
		return 0;

	double aa = fabs(degreesToRadians(set_aa));

	double I = 0.08; 	//TODO: lookup based on time since launch
	double rd = 1.2250 * exp((-9.80665 * 0.0289644 * x)/(8.31432*288.15)); 

	double output = sqrt(fabs(2*aa*I*FINFIT_A)/(rd*v*v*FIN_AREA*FIN_ARM) + FINFIT_B*FINFIT_B) - FINFIT_B;
    output = output / (2*FINFIT_A);

	/* TODO: implement supersonic
    def _supersonic():
        alpha = (aa*I)/(2*rd*v*v*fin_area*fin_arm*Cl_base)
        return degrees(alpha)

    output = 0
    if v <= 265:
        output =  _subsonic()
    elif v < 330:
        # Intepolate between super and subsonic
        y0 = _subsonic()
        y1 = _supersonic()
        x0 = 265
        x1 = 330
        cl = y0 + (y1-y0)*(v - x0)/(x1-x0)
        output =  cl
    else:
        output =  _supersonic()

	*/


    if (set_aa < 0)
        return -output;
    return output;
}

void rc_receive_imu(ADISMessage * imu){

	if (!enable_servo)
		return;

	/* ADIS fixed-to-float conversion to m/s/s */
	const double accel = (0.00333 * 9.80665) * (int16_t) ntohs(imu->data.adis_acc_x);

	/* don't start integrating until it looks like we've launched */
	if (armed && (time_since_launch <= 0 && fabs(accel - 9.80665) < 20 ))
		return;

	const double dt = 1 / 819.2;
	time_since_launch += dt;
	velocity += (accel - 9.80665) * dt;

	/* Proportional Controller */
	// Roll rate:
	const double roll_rate = 0.05 * (int16_t) ntohs(imu->data.adis_gyro_x);


	// Error amd PID Constants
	double error = 0 - roll_rate;
	double proportional = KP * error;
	double correction = proportional;  // TODO: expand to PI or PID

	// Look normilized fin angle based on correctio
	double output = estimate_alpha(correction, 3000, velocity, 0);

	set_canard_angle(output);
}

void rc_receive_arm(const char * signal){
	if(!strcmp(signal, "ARM")){
		set_armed(true);
	}else if(!strcmp(signal, "SAFE")){
		set_armed(false);
		set_canard_angle(0);
	}
}

void rc_raw_ld_in(unsigned char * signal, unsigned int len, unsigned char* timestamp){
	/* we're ignoring launch detect for this launch */
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
			set_servo_enable(true);
			send_servo_response("Roll control servos enabled");
		}
		else if(COMPARE_BUFFER_TO_CMD(data, "DISABLE", len)){
			set_servo_enable(false);
			set_canard_angle(0);
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
