/*
 * Handles the logic of turning sensor data into roll control fin position
 */

#ifndef ROLLCONTROL_H_
#define ROLLCONTROL_H_

#include "utilities/psas_packet.h"

#define MAX_CANARD_ANGLE 15.0            // Canard position in Degrees
#define MIN_CANARD_ANGLE -15.0           // Canard position in Degrees

void rollcontrol_init(void); // [miml:init]
void rc_receive_state(const char *ID, uint8_t *timestamp, uint16_t len, void *buf); // [miml:receiver]
void rc_receive_arm(const char *); // [miml:receiver]
void rc_raw_umb(const char *, unsigned char*, unsigned int, void*); // [miml:receiver]
void rc_raw_testrc(unsigned char *, unsigned int, unsigned char*); // [miml:receiver]


void rc_send_servo(ROLLMessage*); // [miml:sender]

#endif /* ROLLCONTROL_H_ */

