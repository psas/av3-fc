#include <string.h>
#include "filter.h"

void filter_receive(const char ID[4], uint8_t timestamp[6], uint16_t data_length, void *data)
{
	// We log most GPS data to disk only.
	if (!memcmp(ID, "V8", 2) || !memcmp(ID, "JGPS", 4)) {
		// ...with the exception of V8A8, which we want to see in real time.
		if (!memcmp(ID, "V8A8", 4))
			filter_to_ground(ID, timestamp, data_length, data);
		else
			filter_to_disk(ID, timestamp, data_length, data);
	}
	else // Everything else we want in real time.
		filter_to_ground(ID, timestamp, data_length, data);
}
