/*
 *
 */
#include <string.h>
#include <stdio.h>
#include "adis.h"

/*
 *  14/15 = XGYR - 14bit
 *  16/17 = YGYR - 14bit
 *  18/19 = ZGYR - 14bit
 *  20/21 = XACCL - 14bit
 *  22/23 = YACCL - 14bit
 *  24/25 = ZACCL - 14bit
 *  26/27 = XMAG - 14bit
 *  28/29 = YMAG - 14bit
 *  30/31 = ZMAG - 14bit
 *  32/33 = TEMP - 12bit
*/

void adis_raw_in(unsigned char *buffer, int unsigned len, unsigned char* timestamp) {
	//FIXME: this works right now, but is not actually right
	if(len == sizeof(ADIS16405BurstData)){
		// Build message header
		ADISMessage packet ={
				.ID={"ADIS"},
				.timestamp={(uint8_t)timestamp[0], (uint8_t)timestamp[1],
						    (uint8_t)timestamp[2], (uint8_t)timestamp[3],
						    (uint8_t)timestamp[4], (uint8_t)timestamp[5]},
				.data_length=sizeof(ADIS16405BurstData)
		};
		// Copy in data from socket
		memcpy(&packet.data, buffer, sizeof(ADIS16405BurstData));

		adis_data_out(&packet);
	}
	//TODO: else log error?
}


