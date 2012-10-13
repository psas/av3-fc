#ifndef CRESCENT_H
#define CRESCENT_H

#include <stdint.h>

// control Crescent GPS and parse binary packet to print signal strength info

struct msg1 {
	uint8_t		age_of_diff;
	uint8_t		num_sats;
	uint16_t	gps_week;
	double		time_of_week;	// 8 bytes  (but at a 4 byte boundary, so packed)
	double		latitude;		// degrees
	double		longitude;
	float		height;			// meters
	float		v_north;		// meters/s
	float		v_east;
	float		v_up;
	float		sd_residuals;	// meters
	uint16_t	nav_mode;		// 0 no fix, 1 2D fix, 2 3D fix, 3 2D + Diff, 4 3D + Diff, 5 RTK search, 6 3D + Diff + RTK
	uint16_t	ext_age_of_diff;	// if 0, use age_of_diff
} __attribute__((packed));

// cf 7.1.10  Crescent Integrators Manual

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
};

struct msg99 {
	uint8_t		nav_mode;		// 0 invalid, 1 no fix, 2 2D fix, 3 3D fix
	uint8_t		time_diff;		// seconds between GPS and UTC
	uint16_t	gps_week;
	double		time_of_week;	// 8 bytes  (but at a 4 byte boundary, so packed)
	struct chan99 c[12];
	int16_t		clock_err_L1;
	int16_t		spare1;
} __attribute__((packed));

// cf 7.1  Crescent Integrators Manual

struct msg {
	char 		magic[4];		// "$BIN"
	uint16_t	type;			//  1,  2, 80, 93-99
	uint16_t	len;			// 52, 16, 40, 56, 96, 128, 300, 28, 68, 304 
	union {
		char raw[304];
		struct msg1  m1;
		struct msg99 m99;
		// define more of them if we ever need any
	};
};

#endif
