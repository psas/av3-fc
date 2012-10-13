#ifndef AV3_APS_H
#define AV3_APS_H

#include <stdint.h>

#define IOUT_PIN 23
#define ACOK_PIN 11
#define FC_SPS_PIN 5
#define ATV_SPS_PIN 6
#define RC_POWER_PIN 7
#define ROCKET_READY_PIN 8
#define WIFI_POWER_PIN 9
#define RC_TETHER 15

void init_aps(void);
void set_port(int port, uint32_t val);
void clear_port(int port, uint32_t val);

#endif /* AV3_APS_H */
