/*
 * gps.h
 *
 *  Created on: Jun 22, 2013
 *      Author: theo
 */

#ifndef GPS_H_
#define GPS_H_

#include <stdint.h>

/*! \typedef
 * Crescent GPS 1 message
 */

struct msg1 {
        uint8_t         age_of_diff;
        uint8_t         num_sats;
        uint16_t        gps_week;
        double          time_of_week;   // 8 bytes  (but at a 4 byte boundary, so packed)
        double          latitude;               // degrees
        double          longitude;
        float           height;                 // meters
        float           v_north;                // meters/s
        float           v_east;
        float           v_up;
        float           sd_residuals;   // meters
        uint16_t        nav_mode;               // 0 no fix, 1 2D fix, 2 3D fix, 3 2D + Diff, 4 3D + Diff, 5 RTK search, 6 3D + Diff + RTK
        uint16_t        ext_age_of_diff;        // if 0, use age_of_diff
} __attribute__((packed));

/*! \typedef
 * Crescent GPS 99 message
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

typedef struct{
	char         ID[4];			// "GPSX", "GPS1" or "GP99"
	uint8_t      timestamp[6];
	uint16_t     data_length;
	union {
		char raw[304];
		struct msg1  gps1;
		struct msg99 gps99;
	};
} __attribute__((packed)) GPSMessage;


void gps_init(void);
void gps_final(void);

void gps_data_out(GPSMessage *);

#endif /* GPS_H_ */
