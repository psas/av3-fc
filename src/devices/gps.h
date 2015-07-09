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
 
void cots_raw_in(const char *, uint8_t *, uint16_t, void *);
void gps_data_out(const char *, uint8_t *, uint16_t, void *);

#endif /* GPS_H_ */
