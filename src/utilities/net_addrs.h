/* Canonical definition of device addresses on the RocketNet
 * This file should be synched across all projects that want
 * to connect to the RocketNet
 */
#ifndef NET_ADDRS_H_
#define NET_ADDRS_H_
#include <netinet/in.h>

#ifdef FCF_FC_NETWORK
#define RNH_IP    IPv4(10,0,0,5)
#define FC_IP     IPv4(10,0,0,10)
#define SENSOR_IP IPv4(10,0,0,20)
#define ROLL_IP   IPv4(10,0,0,30)
#define WIFI_IP   IPv4(10,0,0,255)
#else
#define RNH_IP    IPv4(127,0,0,1)
#define FC_IP     IPv4(127,0,0,1)
#define SENSOR_IP IPv4(127,0,0,1)
#define ROLL_IP   IPv4(127,0,0,1)
#define WIFI_IP   IPv4(127,0,0,1)
#endif

/* Rocket Net Hub */
#define RNH_RCI 36100     // RCI listener
#define RNH_BATTERY 36101 // Battery data
#define RNH_PORT 36102    // Port data
extern const struct sockaddr * RNH_RCI_ADDR;
extern const struct sockaddr * RNH_BATTERY_ADDR;
extern const struct sockaddr * RNH_PORT_ADDR;

/* Flight Computer */
#define FC_LISTEN_PORT 36000 // FC device listener
#define FCF_HEALTH_PORT 36201 // FC health monitor
extern const struct sockaddr * FC_ADDR;

/* Sensor Node */
#define ADIS_PORT 35020 // ADIS16405
#define MPU_PORT 35002  // MPU1950
#define MPL_PORT 35010  // MPL3115A2
extern const struct sockaddr * ADIS_ADDR;
extern const struct sockaddr * MPU_ADDR;
extern const struct sockaddr * MPL_ADDR;

/* Roll Control */
#define ROLL_PORT 35003    // Servo control
#define TEATHER_PORT 35004 // Launch detect
extern const struct sockaddr * ROLL_ADDR;
extern const struct sockaddr * TEATHER_ADDR;

/* WiFi */
#define WIFI_PORT 35001
extern const struct sockaddr * WIFI_ADDR;

/* ARM Signal */
#define ARM_IP IPv4(127,0,0,1)
#define ARM_PORT 35666
#define RC_SERVO_ENABLE_PORT 35667
extern const struct sockaddr * ARM_ADDR;
extern const struct sockaddr * RC_SERVO_ENABLE_ADDR;

#endif /* NET_ADDRS_H_ */
