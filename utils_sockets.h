/*
 * utils_sockets.h
 *
 *  Created on: Jan 26, 2013
 */

#ifndef NETUTILS_H_
#define NETUTILS_H_

int readsocket(int fd, unsigned char *buffer, int bufsize);
int getsocket(int serverport);

#endif /* NETUTILS_H_ */
