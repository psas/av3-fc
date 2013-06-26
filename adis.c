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

int16_t fix_complement(uint16_t value, int bits) {
	int distance = 16 - bits;
	return ((int16_t)value << distance) >> distance;
}

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
		packet.data.adis_supply_out = buffer[12] << 8 | buffer[13];
		packet.data.adis_xgyro_out = fix_complement(buffer[14] << 8 | buffer[15], 14);
		packet.data.adis_ygyro_out = fix_complement(buffer[16] << 8 | buffer[17], 14);
		packet.data.adis_zgyro_out = fix_complement(buffer[18] << 8 | buffer[19], 14);
		packet.data.adis_xaccl_out = fix_complement(buffer[20] << 8 | buffer[21], 14);
		packet.data.adis_yaccl_out = fix_complement(buffer[22] << 8 | buffer[23], 14);
		packet.data.adis_zaccl_out = fix_complement(buffer[24] << 8 | buffer[25], 14);
		packet.data.adis_xmagn_out = fix_complement(buffer[26] << 8 | buffer[27], 14);
		packet.data.adis_ymagn_out = fix_complement(buffer[28] << 8 | buffer[29], 14);
		packet.data.adis_zmagn_out = fix_complement(buffer[30] << 8 | buffer[31], 14);
		packet.data.adis_temp_out = fix_complement(buffer[32] << 8 | buffer[33], 12);
		packet.data.adis_aux_adc = buffer[34] << 8 | buffer[35];
//		unsigned char* data_section = &(buffer[12]);
//		memcpy(&packet.data, data_section, sizeof(ADIS16405BurstData));
//		printf("%d, %d, %d\n", packet.data.adis_xgyro_out, packet.data.adis_xaccl_out, packet.data.adis_temp_out);
		// Send data out
		adis_data_out(&packet);
	}
	//TODO: else log error?
}


