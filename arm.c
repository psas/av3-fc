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
#include <libusb-1.0/libusb.h>
#include "arm.h"
#include "fcfutils.h"
#include "utils_libusb-1.0.h"

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

/*
static void set_port(int port, uint32_t val){
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
*/
void arm_raw_in(unsigned char *buffer, int len, unsigned char * timestamp){
	printf("\nARMARMARM\n\n\n");
}

void arm_receive_imu(ADISMessage * data){
	// does the acceleration vector == -1g over the last 100 samples?
	// 3.3mg per bit
	double x = data->data.adis_xaccl_out * 0.00333;
	double y = data->data.adis_yaccl_out * 0.00333;
	double z = data->data.adis_zaccl_out * 0.00333;
	add_sample(sqrt(x*x + y*y + z*z));
	return;
}
void arm_receive_gps(GPSMessage * data){
	// check for GPS lock
	if (data->ID[3] == '1') {
		switch(data->gps1.nav_mode) {
		case 2:   // 3D fix
		case 4:   // 3D + diff
		case 6:	  // 3D + diff + rtk
			GPS_locked = 1; break;
		default:
			GPS_locked = 0; break;
		}
	}
	return;
}

void arm_init(void){
	char aps_name[] = "aps";
	init_libusb(aps_name);
	aps = open_device (aps_name, 0xFFFF, 0x0006);
}
void arm_final(void){
	close_device(aps);
	return;
}
