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

void adis_raw_in(unsigned char *buffer, int len, unsigned char* timestamp) {
	if(len == sizeof(ADISMessage)){
		// Build message header
		ADISMessage packet ={
				.ID={"ADIS"},
				.timestamp={(uint8_t)timestamp[0], (uint8_t)timestamp[1],
						    (uint8_t)timestamp[2], (uint8_t)timestamp[3],
						    (uint8_t)timestamp[4], (uint8_t)timestamp[5]},
				.data_length=buffer[10] << 8 | buffer[11]
		};
		// Copy in data from socket
		unsigned char* data_section = &(buffer[12]);
		memcpy(&packet.data, data_section, sizeof(ADIS16405BurstData));

		// Send data out
		adis_data_out(&packet);
	}
	//TODO: else log error?
}

void complement_shift(uint8_t **start, len) {
  int distance = 16 - len;
  uint8_t high = start;
  uint8_t low = start+1;
  start+1 = (high << 2) >> 2
}
