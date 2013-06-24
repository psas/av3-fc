/*
 * ethmux.h
 *
 */

#ifndef ETHMUX_H_
#define ETHMUX_H_


int ethmux_init();
void ethmux_final();
//senders:
void sendARMPacket(unsigned char*, int len);
void sendLDPacket(unsigned char*, int len);
void sendADISPacket(unsigned char*, int len);

#endif /* ETHMUX_H_ */
