/*
 * module_theo-imu.h
 *
 * Initializes and automatically starts data transfers from theo-imu, and logs
 * the recorded data.
 */

#ifndef THEO_IMU_H_
#define THEO_IMU_H_

void init_theo_imu(void); // [miml:init]
void finalize_theo_imu(void); // [miml:final]
extern void sendMessage_theo_imu(const char * src, unsigned char * buff, int length); // [miml:sender]

#endif /* THEO_IMU_H_ */
