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
#include <libusb-1.0/libusb.h>
#include "arm.h"
#include "net_addrs.h"
#include "utils_sockets.h"
#include "fcfutils.h"
#include "utils_libusb-1.0.h"

int response_fd;

static unsigned char buffer[100];

#define IOUT_PIN 23
#define ACOK_PIN 11
#define FC_SPS_PIN 5
#define ATV_SPS_PIN 6
#define RC_POWER_PIN 7
#define ROCKET_READY_PIN 8
#define WIFI_POWER_PIN 9
#define RC_TETHER 15


int GPS_locked;
libusb_device_handle * aps;
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


void set_port(int port, uint32_t val){
    int setport = 0x80;
    unsigned char data[64];
    int usb_err;
    data[0] = (val & 0xFF<<0)>>0;
    data[1] = (val & 0xFF<<8)>>8;
    data[2] = (val & 0xFF<<16)>>16;
    data[3] = (val & 0xFF<<24)>>24;
    usb_err = libusb_control_transfer(aps,
            LIBUSB_RECIPIENT_OTHER | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT,
            setport | port, 0, 0, data, 4, 2000);
    if(usb_err < 0){
//        print_libusb_error(usb_err, "set_port");
    }
    if(usb_err != 4){
        printf("set_port: Didn't send correct number of bytes");
    }
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
			//see aps-host.c in gfe2386/aps/hostside for more functionality
			set_port(0, (1<<ROCKET_READY_PIN));
			sendARMData((char *)buffer);
		}
		// send response to sender (success or why failed)
		if (sendto(response_fd, response, strlen(response)+1, 0, (struct sockaddr *)&sender, addrlen) < 0)
			perror("sendto() response");
	}
}

int arm_init(){
	char aps_name[] = "aps";
	init_libusb(aps_name);
	aps = open_device (aps_name, 0xFFFF, 0x0006);

	int fd = getsocket(ARM_IP, ARM_PORT_S, FC_LISTEN_PORT);
	int rc = fcf_add_fd(fd, POLLIN, arm_signal_cb);
	response_fd = get_send_socket();
	return rc;
}
void arm_final(){
	close_device(aps);
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
	// check for GPS lock
	GPS_locked = (data->data.nav_mode > 1);   // at least 2D fix
	return;
}
