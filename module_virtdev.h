/*
 * module_virtdev.h
 *
 *  Created on: Jan 26, 2013
 */

#ifndef VIRTDEVSRV_H_
#define VIRTDEVSRV_H_

void init_virtdev(void); // [miml:init]
void finalize_virtdev(void); // [miml:final]
extern void sendMessage_virtdev(const char * src, unsigned char *buffer, int length); // [miml:sender]

#endif /* VIRTDEVSRV_H_ */
