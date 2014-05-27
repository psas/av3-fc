/*
 * Interprets messages from the ADIS16405 IMU
 */

#ifndef ADIS_H_
#define ADIS_H_

#include <stdint.h>
#include "utilities/psas_packet.h"

void MIML_SENDER adis_data_out(ADISMessage*);
void MIML_RECEIVER adis_raw_in(unsigned char*, unsigned int, unsigned char*);

#endif /* ADIS_H_ */
