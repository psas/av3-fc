
#include <stdio.h>
#include <unistd.h>
#include "rollControlLibrary.h"
#include "rollservo.h"
#include "net_addrs.h"
#include "utils_sockets.h"

int sd;

void rollservo_init(void){
	sd = get_send_socket();
}

void rollservo_final(void){
	close(sd);
}

void rs_getPositionData_rc(RollServo_adjustment* adj){
	char data[3];
	data[0] = (0xff00 & adj->u16ServoPulseWidthBin14) >> 8;
	data[1] = (0xff & adj->u16ServoPulseWidthBin14);
	data[2] = adj->u8ServoDisableFlag;
	sendto_socket(sd, data, sizeof(data), ROLL_IP, ROLL_PORT);
	sendRollServoData(data);
}



