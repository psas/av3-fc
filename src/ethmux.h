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
extern void demuxed_ADIS(const char *, uint8_t *, uint16_t, void *);
extern void demuxed_MPU(const char *, uint8_t *, uint16_t, void *);
extern void demuxed_MPL(const char *, uint8_t *, uint16_t, void *);
extern void demuxed_RC(unsigned char*, unsigned int, unsigned char*);
extern void demuxed_RNH(const char *, uint8_t *, uint16_t, void *);
extern void demuxed_FCFH(const char *, uint8_t *, uint16_t, void *);
extern void demuxed_COTS(const char *, uint8_t *, uint16_t, void *);
void sequenced_error(const char *, uint8_t *, uint16_t, void *);

#endif /* ETHMUX_H_ */
