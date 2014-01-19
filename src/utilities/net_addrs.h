#ifndef NET_ADDRS_H_
#define NET_ADDRS_H_

// FLIGHT COMPUTER
#ifdef FCF_FC_NETWORK
#define FC_IP "10.0.0.10"
#else
#define FC_IP "127.0.0.1"
#endif
#define FC_LISTEN_PORT 36000

// WIFI
#ifdef FCF_FC_NETWORK
#define WIFI_IP "172.17.0.255"
#else
#define WIFI_IP "127.0.0.1"
#endif
#define WIFI_PORT 35001

// Sensor Node
#define SENSOR_IP "10.0.0.20"
#define ADIS_RX_PORT 35020 // ADIS
#define MPU_RX_PORT 35002  // MPU1950
#define MPL_RX_PORT 35010  // MPL3115A2

// ROLL
#ifdef FCF_FC_NETWORK
#define ROLL_IP "10.0.0.30"
#else
#define ROLL_IP "127.0.0.1"
#endif
#define ROLL_TX_PORT 35003
#define ROLL_RX_PORT 35004
#define TEATHER_PORT 35004 // ROLL TEATHER SIGNAL

// ARM SIGNAL
#define ARM_IP "127.0.0.1"
#define ARM_PORT 35666

#define RC_SERVO_ENABLE_PORT 35667

#endif /* NET_ADDRS_H_ */
