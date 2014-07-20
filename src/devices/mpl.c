#include <string.h>
#include <stdint.h>
#include "utils_sockets.h"
#include "mpl.h"

void mpl_raw_in(unsigned char *buffer, unsigned int len, unsigned char* timestamp) {
	if(len == sizeof(MPLData)){
		MPLMessage packet ={
			.ID={"MPL3"},
			.timestamp={(uint8_t)timestamp[0], (uint8_t)timestamp[1],
			            (uint8_t)timestamp[2], (uint8_t)timestamp[3],
			            (uint8_t)timestamp[4], (uint8_t)timestamp[5]},
			.data_length=htons(len)
		};
		memcpy(&packet.data, buffer, len);

		mpl_data_out(&packet);
	}
}
