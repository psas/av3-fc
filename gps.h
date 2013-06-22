/*
 * gps.h
 *
 *  Created on: Jun 22, 2013
 *      Author: theo
 */

#ifndef GPS_H_
#define GPS_H_

typedef struct{
	char data;
} GPS_packet;

void sendGPSData(GPS_packet *);

#endif /* GPS_H_ */
