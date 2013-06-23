/*
 * ethmux.h
 *
 */

#ifndef ETHMUX_H_
#define ETHMUX_H_


void ethmux_init();
void ethmux_final();
//senders:
void sendARMData(char*);
void sendLDData(char*);
void sendADISPacket(char*);

#endif /* ETHMUX_H_ */
