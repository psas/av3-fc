#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "gps.h"




void cots_raw_in(unsigned char* buffer, unsigned int len, unsigned char* timestamp) {
	printf("%d, ", (uint16_t)buffer[4]);

	GPSFixData data;
	switch(len - 11){
	case sizeof(GPSFixData):
		memcpy(&data, buffer + 8, sizeof(GPSFixData));
		printf("GPSFixData, NOS:%d, NAVM: %d, LAT: %f, LON: %f, H: %f, VN: %f, VE: %f, VU: %f", data.gps1_num_of_sats, data.gps1_nav_mode, data.gps1_latitude, data.gps1_longitude, data.gps1_height, data.gps1_vnorth, data.gps1_veast, data.gps1_vup);
		break;

	case sizeof(GPSFixQualityData):
		printf("GPSFixQualityData, ");
			break;

	case sizeof(GPS2Message):
		printf("GPS2Message, ");
		break;

	case sizeof(GPSWAASMessageData):
		printf("GPSWAASMessageData, ");
		break;

	case sizeof(GPSWAASEphemerisData):
		printf("GPSWAASEphemerisData, ");
		break;

	case sizeof(GPSEphemerisData):
		printf("GPSEphemerisData, ");
		break;

	case sizeof(GPS95Message):
		printf("GPS95Message, ");
		break;

	case sizeof(GPSPsudorangeData):
		printf("GPSPsudorangeData, ");
		break;

	case sizeof(GPS96Message):
		printf("GPS96Message, ");
		break;

	case sizeof(GPSAlmanacData):
		printf("GPSAlmanacData, ");
		break;

	case sizeof(GPS98Message):
		printf("GPS98Message, ");
		break;

	case sizeof(GPSSatelliteData):
		printf("GPSSatelliteData, ");
		break;

	default:
		printf("Unknown, ");
	}
	printf("\n");
}





