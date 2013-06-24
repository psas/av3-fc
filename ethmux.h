/*
 * ethmux.h
 *
 */

#ifndef ETHMUX_H_
#define ETHMUX_H_


int ethmux_init();
void ethmux_final();
//senders:
void sendARMPacket(unsigned char*, int, unsigned char*);
void sendLDPacket(unsigned char*, int, unsigned char*);
void sendADISPacket(unsigned char*, int, unsigned char*);
void sendMPUPacket(unsigned char*, int, unsigned char*);
void sendMPLPacket(unsigned char*, int, unsigned char*);

#endif /* ETHMUX_H_ */
