#include <stdio.h>
#include <unistd.h>
#include "net_addrs.h"
#include "utils.h"
#include "utils_sockets.h"
#include "rollservo.h"

static int sd;

void rollservo_init(void){
	sd = udp_socket();
	if(sd < 0){
		return;
	}

	if(connect(sd, ROLL_ADDR, sizeof(struct sockaddr_in)) < 0){
		perror("rollservo_init: connect() failed");
		close(sd);
	}
}

void rollservo_final(void){
	close(sd);
}


#define PWM_TICKS_MAX 56000
#define PWM_TICKS_MIN 0
#define PWM_US_MAX 3333
#define PWM_US_MIN 0

#define PWM_TICKS_TO_US ( PWM_US_MAX / PWM_TICKS_MAX)
#define PWM_US_TO_TICKS ( PWM_TICKS_MAX / PWM_US_MAX)

#define MAX_SERVO_POSITION_US 1900
#define MIN_SERVO_POSITION_US 1100

#define MAX_SERVO_POSITION_TICKS ( MAX_SERVO_POSITION_US * PWM_US_TO_TICKS )
#define MIN_SERVO_POSITION_TICKS ( MIN_SERVO_POSITION_US * PWM_US_TO_TICKS )


void rs_receive_adj(RollServoMessage* adj){
	static uint32_t seq = 0;
	/* Slope */
	const double PWM_TICKS_PER_DEGREE = (MAX_SERVO_POSITION_TICKS - MIN_SERVO_POSITION_TICKS) / (MAX_CANARD_ANGLE - MIN_CANARD_ANGLE);
	/* Intercept */
	const double PWM_TICKS_CENTER = MIN_SERVO_POSITION_TICKS - PWM_TICKS_PER_DEGREE * MIN_CANARD_ANGLE;

	uint16_t ticks = PWM_TICKS_PER_DEGREE * adj->finangle + PWM_TICKS_CENTER;
	CLAMP(ticks, MAX_SERVO_POSITION_TICKS, MIN_SERVO_POSITION_TICKS);

	char data[7];
	data[0] = seq >> 24;
	data[1] = seq >> 16;
	data[2] = seq >> 8;
	data[3] = seq;
	data[4] = ticks >> 8;
	data[5] = ticks;
	data[6] = adj->servoDisableFlag;
	if(write(sd, data, sizeof(data)) != sizeof(data)){
		perror("rs_receive_adj: write failed");
	}
	++seq;
}

