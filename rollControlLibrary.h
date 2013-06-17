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
	uint16_t u16RawAccelerometerADC;
	uint16_t u16RawRateGyroADC;
	// Bool, 1 if launched, 0 if not
	// resetting (go back to 0) will restart mission
	uint8_t u8IsLaunchDetected;
} RC_INPUT_STRUCT_TYPE;

typedef struct {
	// Servo ON-Time in milliseconds x 2^14
	// Example: 1.5 msec = 1.5 x 2^14 = 24576
	uint16_t u16ServoPulseWidthBin14;
} RC_OUTPUT_STRUCT_TYPE;

// Call once to initialize model
void rc_init( void );

// Call once every 1 millisecond after initialization
void rc_step( RC_INPUT_STRUCT_TYPE * rc_input_st,
		RC_OUTPUT_STRUCT_TYPE * rc_output_st );


#endif /* ROLLCONTROLLIBRARY_H_ */
