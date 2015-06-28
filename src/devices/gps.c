#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "gps.h"


/**
 * Recieves a COTS GPS message from the network
 */
void cots_raw_in(unsigned char *data, unsigned int len, unsigned char *timestamp)
{
	// Construct a venus message
	V6NAMessage p = {
		.ID = "V6NA",
		.data_length = sizeof(Venus6FixData),
	};
	memcpy(&p.timestamp, timestamp, sizeof(p.timestamp));

	// Copy in message off the wire as data
	memcpy(&p.data, data, len);

	// Send data out to consumers
	gps_data_out(&p);
}
