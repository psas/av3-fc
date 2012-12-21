/*
 * adis-imu.h
 *
 * Initializes and automatically starts data transfers from adis-imu, and logs
 * the recorded data.
 */

#ifndef ADIS_IMU_H_
#define ADIS_IMU_H_

#include "libusb-gsource.h"

void init_adis_imu(libusbSource * usb_source);

#endif /* ADIS_IMU_H_ */
