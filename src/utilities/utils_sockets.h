/*
 * Utilites for reducing boilerplate when dealing with sockets
 */

#ifndef NETUTILS_H_
#define NETUTILS_H_

#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>

#define ETH_MTU 1500

int readsocket(int fd, unsigned char *buffer, int bufsize);
int readsocketfrom(int fd, unsigned char *buffer, int bufsize, struct sockaddr *src, socklen_t *addrlen);
int readsocketfromts(int fd, unsigned char *buffer, int bufsize, struct sockaddr_in *sender, socklen_t addrlen, struct timespec *);
int sendto_socket(int sd, const char *buffer, int bufsize, const char *dest_ip, int dest_port);
int udp_socket();
int bound_udp_socket(int port);
int timestamped_bound_udp_socket(int port);

#endif /* NETUTILS_H_ */
