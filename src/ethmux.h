/*
 * Demuxes ethernet messages sent to the listen port by source and routes
 * them to the appropriate module.
 */

#ifndef ETHMUX_H_
#define ETHMUX_H_
#include <ev.h>

void MIML_INIT ethmux_init(struct ev_loop * loop);
void MIML_FINAL ethmux_final(void);
//senders:
void MIML_SENDER demuxed_ARM(unsigned char*, unsigned int, unsigned char*);
void MIML_SENDER demuxed_LD(unsigned char*, unsigned int, unsigned char*);
void MIML_SENDER demuxed_ADIS(unsigned char*, unsigned int, unsigned char*);
void MIML_SENDER demuxed_MPU(unsigned char*, unsigned int, unsigned char*);
void MIML_SENDER demuxed_MPL(unsigned char*, unsigned int, unsigned char*);
void MIML_SENDER demuxed_RC(unsigned char*, unsigned int, unsigned char*);
void MIML_SENDER demuxed_RNH(unsigned char*, unsigned int, unsigned char*);
void MIML_SENDER demuxed_RNHPORT(unsigned char*, unsigned int, unsigned char*);
void MIML_SENDER demuxed_FCFH(unsigned char*, unsigned int, unsigned char*);

#endif /* ETHMUX_H_ */
