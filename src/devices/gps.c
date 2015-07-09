#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "gps.h"


/**
 * Recieves a COTS GPS message from the network
 */
void cots_raw_in(const char ID[4], uint8_t timestamp[6], uint16_t data_length, void *buffer)
{
	// TODO: instead of logging the raw byte stream, identify frame boundaries and emit only valid frames.
	gps_data_out(ID, timestamp, data_length, buffer);
}
