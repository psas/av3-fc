/*! \file psas_packet.h
 *
 * \sa http://psas.pdx.edu/avionics/av3-data-protocol/
 */

#ifndef _PSAS_PACKET_H
#define _PSAS_PACKET_H

#include "ADIS16405.h"

/*!
 * \addtogroup psaspacket
 * @{
 */

struct ADIS_packet {
    char                 ID[4];
    uint8_t              timestamp[16];
    uint16_t             data_length;
    ADIS16405_burst_data data;
} __attribute__((packed));
typedef struct ADIS_packet ADIS_packet;

#endif
//! @}


