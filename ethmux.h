/*
 * ethmux.h
 *
 */

#ifndef ETHMUX_H_
#define ETHMUX_H_

extern void ethmux_init(void);
extern void ethmux_final(void);
//senders:
extern void demuxed_ARM(unsigned char*, int, unsigned char*);
extern void demuxed_LD(unsigned char*, int, unsigned char*);
extern void demuxed_ADIS(unsigned char*, int, unsigned char*);
extern void demuxed_MPU(unsigned char*, int, unsigned char*);
extern void demuxed_MPL(unsigned char*, int, unsigned char*);

#endif /* ETHMUX_H_ */
