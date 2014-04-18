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


typedef struct {
    uint16_t Temperature;
    int16_t TS1Temperature;
    int16_t TS2Temperature;
    uint16_t TempRange;
    uint16_t Voltage;
    int16_t Current;
    int16_t AverageCurrent;
    uint16_t CellVoltage1;
    uint16_t CellVoltage2;
    uint16_t CellVoltage3;
    uint16_t CellVoltage4;
    uint16_t PackVoltage;
    uint16_t AverageVoltage;
} __attribute__((packed)) RNH_Health_Data;

typedef struct {
    char                 ID[4];			// "RNHH"
    uint8_t              timestamp[6];
    uint16_t             data_length;
    RNH_Health_Data      data;
} __attribute__((packed)) RNHMessage;

typedef struct {
    char                 ID[4];         // "RNHP"
    uint8_t              timestamp[6];
    uint16_t             data_length;
    uint16_t             data[8];
} __attribute__((packed)) RNHPortMessage;


#endif
//! @}


