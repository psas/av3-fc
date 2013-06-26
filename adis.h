/*
 * adis.h
 *
 */

#ifndef ADIS_H_
#define ADIS_H_

#include <stdint.h>

typedef     uint16_t                          adis_reg_data;

/*! \typedef
 * Burst data collection
 */
typedef struct {
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
} __attribute__((packed)) ADIS16405BurstData;

typedef struct {
    char                 ID[4];			// "ADIS"
    uint8_t              timestamp[6];
    uint16_t             data_length;
    ADIS16405BurstData data;
} __attribute__((packed)) ADISMessage;

extern void adis_data_out(ADISMessage*); // [miml:sender]

extern void adis_raw_in(unsigned char*, int, unsigned char*); // [miml:receiver]


#endif /* ADIS_H_ */
