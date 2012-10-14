/*
 * theo-imu.h
 *
 * Initializes and automatically starts data transfers from theo-imu, and logs
 * the recorded data.
 */

#ifndef THEO_IMU_H_
#define THEO_IMU_H_

#include "libusb-gsource.h"

void init_theo_imu(libusbSource * usb_source);

#endif /* THEO_IMU_H_ */
