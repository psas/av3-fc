/*
 * Interprets messages from the ADIS16405 IMU
 */

#ifndef ADIS_H_
#define ADIS_H_

#include <stdint.h>
#include "utilities/psas_packet.h"

extern void adis_data_out(ADISMessage*); // [miml:sender]
extern void adis_raw_in(unsigned char*, unsigned int, unsigned char*); // [miml:receiver]

#endif /* ADIS_H_ */
