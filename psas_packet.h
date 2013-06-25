/*! \file psas_packet.h
 *
 * \sa http://psas.pdx.edu/avionics/av3-data-protocol/
 */

#ifndef _PSAS_PACKET_H
#define _PSAS_PACKET_H

#include <stdint.h>
/*!
 * \addtogroup psaspacket
 * @{
 */

typedef struct{
	char     ID[4];
	uint8_t  timestamp[6];
	uint16_t data_length;
} __attribute__((packed)) message_header;

typedef struct{
	char     ID[4];
	uint8_t  timestamp[6];
	uint16_t data_length;
}__attribute__((packed)) PSASMessage;

#endif
//! @}


