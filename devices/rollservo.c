
#include <unistd.h>
#include "net_addrs.h"
#include "../elderberry/utils_sockets.h"
#include "rollservo.h"

int sd;

void rollservo_init(void){
	sd = get_send_socket();
}

void rollservo_final(void){
	close(sd);
}

void rs_receive_adj(RollServoMessage* adj){
	char data[3];
	data[0] = (0xff & adj->u16ServoPulseWidthBin14);
	data[1] = (0xff00 & adj->u16ServoPulseWidthBin14) >> 8;
	data[2] = adj->u8ServoDisableFlag;
	sendto_socket(sd, data, sizeof(data), ROLL_IP, ROLL_TX_PORT);
}
