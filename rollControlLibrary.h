/*
 * rollControlLibrary.h
 *
 *  Created on: Jun 16, 2013
 *      Author: psas
 */

#ifndef ROLLCONTROLLIBRARY_H_
#define ROLLCONTROLLIBRARY_H_

#include <stdint.h>

typedef struct {
	// Accelerometer - -1 g (on pad) at approx 34500 counts
	// 1374.05 counts / g (check signs)
	uint16_t u16RawAccelerometerADC;

	// Rate Gyro - Zero @ approx 29400 counts
	// 43.69 counts / degree/second (check signs)
	uint16_t u16RawRateGyroADC;

	// Bool, 1 if launched, 0 if not
	// resetting (go back to 0) will restart mission
	uint8_t u8IsLaunchDetected;
} RC_INPUT_STRUCT_TYPE;

typedef struct {
	// Servo ON-Time in milliseconds x 2^14
	// Example: 1.5 msec = 1.5 x 2^14 = 24576
	uint16_t u16ServoPulseWidthBin14;

	// Disable servo (turn off PWM) when this flag is not 0
	uint8_t u8ServoDisableFlag;
}RC_OUTPUT_STRUCT_TYPE;

// Call once to initialize model
void rc_init( void );

// Call once every 1 millisecond after initialization
void rc_step( RC_INPUT_STRUCT_TYPE * rc_input_st,
		RC_OUTPUT_STRUCT_TYPE * rc_output_st );


#endif /* ROLLCONTROLLIBRARY_H_ */
