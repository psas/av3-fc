/*
 * Manages and interprets messages from the Crescent GPS device
 */
#ifndef GPS_H_
#define GPS_H_

#include <stdint.h>
#include "psas_packet.h"
// chan99.status bitfields
#define STATUS99_CODE_LOCK	0x01
#define STATUS99_BIT_LOCK	0x02
#define STATUS99_FRAME_LOCK	0x04
#define STATUS99_FRAME_SYNC	0x08
#define STATUS99_FAME_SYNC_NEW_EPOCH	0x10
#define STATUS99_CHANNEL_RESET	0x20
#define STATUS99_PHASE_LOCK	0x40

#define GPSMessage GPS1Message

void cots_raw_in(unsigned char*, unsigned int, unsigned char*);
void gps_data_out(GPSMessage *);

#endif /* GPS_H_ */
