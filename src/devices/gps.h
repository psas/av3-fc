/*
 * Manages and interprets messages from the Crescent GPS device
 */
#ifndef GPS_H_
#define GPS_H_

#include <stdint.h>
#include "psas_packet.h"

// Venus 6 fix mode bitmask:
#define VENUS6_FIX_MODE_NO_FIX 0
#define VENUS6_FIX_MODE_2D 1
#define VENUS6_FIX_MODE_3D 2
#define VENUS6_FIX_MODE_3D_DGPS 3
 
void cots_raw_in(unsigned char*, unsigned int, unsigned char*);
void gps_data_out(V6NAMessage *);

#endif /* GPS_H_ */
