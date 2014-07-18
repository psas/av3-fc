/*
 * Demuxes ethernet messages sent to the listen port by source and routes
 * them to the appropriate module.
 */

#ifndef ETHMUX_H_
#define ETHMUX_H_

extern void ethmux_init(void);
extern void ethmux_final(void);
//senders:
extern void demuxed_ARM(unsigned char*, unsigned int, unsigned char*);
extern void demuxed_LD(unsigned char*, unsigned int, unsigned char*);
extern void demuxed_ADIS(unsigned char*, unsigned int, unsigned char*);
extern void demuxed_MPU(unsigned char*, unsigned int, unsigned char*);
extern void demuxed_MPL(unsigned char*, unsigned int, unsigned char*);
extern void demuxed_RC(unsigned char*, unsigned int, unsigned char*);
extern void demuxed_RNH(unsigned char*, unsigned int, unsigned char*);
extern void demuxed_FCFH(unsigned char*, unsigned int, unsigned char*);
extern void demuxed_COTS(unsigned char*, unsigned int, unsigned char*);
void sequenced_error(unsigned short port, uint8_t * buffer, unsigned int len, uint8_t * timestamp, uint32_t expected, uint32_t received);

#endif /* ETHMUX_H_ */
