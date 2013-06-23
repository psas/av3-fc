/*
 * arm.c
 *
 *  Created on: Jun 22, 2013
 *      Author: theo
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "arm.h"
#include "net_addrs.h"
#include "utils_sockets.h"
#include "fcfutils.h"

int response_fd;

static unsigned char buffer[100];

int GPS_locked;

#define SAMPLES 100
static double acceleration[SAMPLES], *cur = acceleration, sum = 0.0;

// running average
static void add_sample(double a)
{
	sum = sum - *cur + a;
	*cur++ = a;
	if (cur == acceleration + SAMPLES)
		cur = acceleration;
}

static void arm_signal_cb(struct pollfd *pfd){
	struct sockaddr_storage sender;
	socklen_t addrlen = sizeof(sender);
	const char *response = "You Only Launch Once";
	int rc = readsocketfrom(pfd->fd, buffer, sizeof(buffer), (struct sockaddr *)&sender, &addrlen);
	if(rc > 0){
		// is it the ARM signal?  ("#YOLO")
		if (memcmp(buffer, "#YOLO", 6) != 0)
			response = "Unrecognized ARM command!";
		// is adis OK (-1G)
		else if (fabs(sum/SAMPLES - 1.0) > 0.001)
			response = "Accelerometers out of limits!";
		// is GPS locked?
		else if (!GPS_locked)
			response = "GPS not locked!";
		else {
			//TODO: set RocketReady high
			sendARMData((char *)buffer);
		}
		// send response to sender (success or why failed)
		if (sendto(response_fd, response, strlen(response)+1, 0, (struct sockaddr *)&sender, addrlen) < 0)
			perror("sendto() response");
	}
}

int arm_init(){
	int fd = getsocket(ARM_IP, ARM_PORT_S, FC_LISTEN_PORT);
	int rc = fcf_add_fd(fd, POLLIN, arm_signal_cb);
	response_fd = get_send_socket();
	return rc;
}
void arm_final(){
	close(response_fd);
	return;
}

static double raw2g(uint16_t raw)
{
	// 3.3mg per bit, 14 bits per axis
	// conversions: cf. stm32/src/si/host_fc/data-analysis/adis16405/adis_convert.py
	int16_t d = raw;
	if (raw & 0x2000)
		d |= 0xc000;	// sign-extend 14 -> 16 bits
	return d * 0.00333;
}

void arm_getPositionData_adis(ADIS_packet * data){
	// does the acceleration vector == -1g over the last 100 samples?
	double x = raw2g(data->data.adis_xaccl_out);
	double y = raw2g(data->data.adis_yaccl_out);
	double z = raw2g(data->data.adis_zaccl_out);
	add_sample(sqrt(x*x + y*y + z*z));
	return;
}
void arm_getPositionData_gps(GPS_packet * data){
	//TODO: check for GPS lock
	return;
}
