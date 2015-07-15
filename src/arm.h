/*
 * ARM = ARM Rocket Manager
 * Manages global rocket state
 */

#ifndef ARM_H_
#define ARM_H_
#include <stdint.h>
#include "psas_packet.h"

void arm_init(void);
void arm_final(void);

void arm_receive_imu(const char *ID, uint8_t *timestamp, uint16_t len, void *buf);
void arm_receive_gps(const char *, uint8_t *, uint16_t, void *);
void arm_raw_in(unsigned char *, unsigned int, unsigned char *);

void arm_send_signal(const char *);
#endif /* ARM_H_ */
