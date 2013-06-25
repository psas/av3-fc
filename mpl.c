/*
 * mpl.c
 *
 */

#include <string.h>
#include <stdint.h>
#include "mpl.h"

void mpl_raw_in(unsigned char *buffer, int len, unsigned char* timestamp) {
	if(len == sizeof(MPLMessage)){
		// Build message header
		MPLMessage packet ={ .ID={buffer[0], buffer[1], buffer[2], buffer[3]},
							  .timestamp={(uint8_t*)timestamp},
							  .data_length=buffer[10] << 8 | buffer[11]
		};
		// Copy in data from socket
		packet.data = buffer;
//		unsigned char* data_section = &(buffer[12]);
//		memcpy(&packet.data, data_section, sizeof(MPL_burst_data));

		// Send data out
		mpl_data_out(&packet);
	}
	// TODO: else log error?
}
