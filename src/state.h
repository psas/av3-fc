/**
 * State estimation on the rocket
 */

#ifndef STATE_H_
#define STATE_H_

#include "utilities/psas_packet.h"

#define EARTHG 9.80665
#define ADIS_GLSB  0.00333 * EARTHG
#define ADIS_RLSB 0.05
#define LAUNCH_ALTITUDE 1390.0

void state_init(void); // [miml:init]
void state_receive_imu(ADISMessage *); // [miml:receiver]
void state_raw_ld_in(unsigned char *, unsigned int, unsigned char*); // [miml:receiver]
void state_send_message(VSTEMessage *); // [miml:sender]


#endif /* STATE_H_ */
