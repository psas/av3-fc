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

typedef     uint16_t                          adis_reg_data;

/*! \typedef
 * Burst data collection
 */
struct ADIS16405_burst_data {
	adis_reg_data      adis_supply_out; //  Power supply measurement
	adis_reg_data      adis_xgyro_out;  //  X-axis gyroscope output
	adis_reg_data      adis_ygyro_out;  //  Y-axis gyroscope output
	adis_reg_data      adis_zgyro_out;  //  Z-axis gyroscope output
	adis_reg_data      adis_xaccl_out;  //  X-axis accelerometer output
	adis_reg_data      adis_yaccl_out;  //  Y-axis accelerometer output
	adis_reg_data      adis_zaccl_out;  //  Z-axis accelerometer output
	adis_reg_data      adis_xmagn_out;  //  X-axis magnetometer measurement
	adis_reg_data      adis_ymagn_out;  //  Y-axis magnetometer measurement
	adis_reg_data      adis_zmagn_out;  //  Z-axis magnetometer measurement
	adis_reg_data      adis_temp_out;   //  Temperature output
	adis_reg_data      adis_aux_adc;    //  Auxiliary ADC measurement
} __attribute__((packed));
typedef struct ADIS16405_burst_data ADIS16405_burst_data;

struct ADIS_packet {
    char                 ID[4];
    uint8_t              timestamp[16];
    uint16_t             data_length;
    ADIS16405_burst_data data;
} __attribute__((packed));
typedef struct ADIS_packet ADIS_packet;

#endif
//! @}


