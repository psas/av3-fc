/*
 * utils_sockets.h
 *
 *  Created on: Jan 26, 2013
 */

#ifndef NETUTILS_H_
#define NETUTILS_H_

/*typedef struct socksetup {
	int fd;
	struct sockaddr_in *addr;
} socksetup;*/

int readsocket(int fd, unsigned char *buffer, int bufsize);
int sendto_socket(int sd, char *buffer, int bufsize, const char *dest_ip, int dest_port);
int get_send_socket();
int getsocket(const char *source_ip, const char *source_port, int listen_port);

#endif /* NETUTILS_H_ */
