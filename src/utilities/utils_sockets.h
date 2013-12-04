/*
 * sockets.h
 *
 *  Created on: Jan 26, 2013
 */

#ifndef NETUTILS_H_
#define NETUTILS_H_

#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>

int readsocket(int fd, unsigned char *buffer, int bufsize);
int readsocketfrom(int fd, unsigned char *buffer, int bufsize, struct sockaddr *src, socklen_t *addrlen);
int readsocketfromts(int fd, unsigned char *buffer, int bufsize, struct sockaddr_in *sender, socklen_t addrlen, struct timespec *);
int sendto_socket(int sd, const char *buffer, int bufsize, const char *dest_ip, int dest_port);
int get_send_from_socket(int send_port);
int get_send_socket();
int getsocket(const char *source_ip, const char *source_port, int listen_port);

#endif /* NETUTILS_H_ */
