/*
 * virtdevsrv.h
 *
 *  Created on: Jan 26, 2013
 */

#ifndef VIRTDEVSRV_H_
#define VIRTDEVSRV_H_

extern int init_virt###DEVNAME###(void); // [miml:init]
extern void finalize_virt###DEVNAME###(void); // [miml:final]
extern void sendMessage_virt###DEVNAME###(const char *src, unsigned char *buffer, int length); // [miml:sender]

#endif /* VIRTDEVSRV_H_ */
