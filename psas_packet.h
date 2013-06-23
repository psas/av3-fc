/*! \file psas_packet.h
 *
 * \sa http://psas.pdx.edu/avionics/av3-data-protocol/
 */

#ifndef _PSAS_PACKET_H
#define _PSAS_PACKET_H

#include <stdint.h>
/*!
 * \addtogroup psaspacket
 * @{
 */

struct packet_header {
	char     ID[4];
	uint8_t  timestamp[6];
	uint16_t data_length;
} __attribute__((packed));
typedef struct packet_header packet_header;


typedef     uint16_t                          adis_reg_data;

/*! \typedef
 * Burst data collection
 */
struct ADIS16405_burst_data {
	adis_reg_data      adis_supply_out; //  Power supply measurement
	adis_reg_data      adis_xgyro_out;  //  X-axis gyroscope output
	adis_reg_data      adis_ygyro_out;  //  Y-axis gyroscope output
	adis_reg_data      adis_zgyro_out;  //  Z-axis gyroscope output
	adis_reg_data      adis_xaccl_out;  //  X-axis accelerometer output
	adis_reg_data      adis_yaccl_out;  //  Y-axis accelerometer output
	adis_reg_data      adis_zaccl_out;  //  Z-axis accelerometer output
	adis_reg_data      adis_xmagn_out;  //  X-axis magnetometer measurement
	adis_reg_data      adis_ymagn_out;  //  Y-axis magnetometer measurement
	adis_reg_data      adis_zmagn_out;  //  Z-axis magnetometer measurement
	adis_reg_data      adis_temp_out;   //  Temperature output
	adis_reg_data      adis_aux_adc;    //  Auxiliary ADC measurement
} __attribute__((packed));
typedef struct ADIS16405_burst_data ADIS16405_burst_data;

struct ADIS_packet {
    char                 ID[4];			// "ADIS"
    uint8_t              timestamp[6];
    uint16_t             data_length;
    ADIS16405_burst_data data;
} __attribute__((packed));
typedef struct ADIS_packet ADIS_packet;


/*! \typedef
 * Crescent GPS 99 packet
 */

// chan99.status bitfields
#define STATUS99_CODE_LOCK	0x01
#define STATUS99_BIT_LOCK	0x02
#define STATUS99_FRAME_LOCK	0x04
#define STATUS99_FRAME_SYNC	0x08
#define STATUS99_FAME_SYNC_NEW_EPOCH	0x10
#define STATUS99_CHANNEL_RESET	0x20
#define STATUS99_PHASE_LOCK	0x40

struct chan99 {
	uint8_t		channel;
	uint8_t		sat;			// 0 not tracked
	uint8_t		status;
	uint8_t		last_sub_frame;
	uint8_t		ephem_valid;
	uint8_t		ephem_health;
	uint8_t		almanac_valid;
	uint8_t		almanac_health;
	int8_t		elevation;
	uint8_t		half_azimuth;	// 0-180 for 0-360 degrees
	uint8_t		user_range_error;
	uint8_t		spare1;			// unused
	uint16_t	cli_no;			// SNR = 10 * 4096 * cli_no / 80000  (noise floor)
	int16_t		diff_corr;		// 100 * differential correction
	int16_t		pos_residual;	// 10 * position residual
	int16_t		vel_residual;	// 10 * velocity residual
	int16_t		doppler_hz;		// expected doppler
	int16_t		carrier_track_offset;
} __attribute__((packed));

struct msg99 {
	uint8_t		nav_mode;		// 0 invalid, 1 no fix, 2 2D fix, 3 3D fix
	uint8_t		time_diff;		// seconds between GPS and UTC
	uint16_t	gps_week;
	double		time_of_week;	// 8 bytes  (but at a 4 byte boundary, so packed)
	struct chan99 c[12];
	int16_t		clock_err_L1;
	int16_t		spare1;
} __attribute__((packed));

struct GPS_packet {
	char         ID[4];			// "GPS9"
	uint8_t      timestamp[6];
	uint16_t     data_length;
	struct msg99 data;
} __attribute__((packed));
typedef struct GPS_packet GPS_packet;

#endif
//! @}


