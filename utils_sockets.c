/**	starting point for the socket code was:
*	http://publib.boulder.ibm.com/infocenter/iseries/v6r1m0/index.jsp?topic=/rzab6/poll.htm
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include "utils_sockets.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>


int readsocket(int fd, unsigned char *buffer, int bufsize) {
	/**
	* Receive data on this connection until the
	* recv fails with EWOULDBLOCK. If any other 
	* failure occurs, we will close the
	* connection. 
	*/
	int rc = recv(fd, buffer, bufsize, 0);
	if (rc < 0){
		if (errno != EWOULDBLOCK){
			perror("readsocket: recv() failed");
			return -2;
		}
		return 0;
	}

	/**
	* Check to see if the connection has been
	* closed by the client 
	*/
	if (rc == 0){
		return -1;
	}
	
	// Data was received 
	//printf("  %d bytes received\n", rc);

	return rc;
}

int sendto_socket(int sd, char *buffer, int bufsize, const char *dest_ip, int dest_port) {

	//printf("%s (%d) - on socket %d msg with %d\n", dest_ip, dest_port, sd, bufsize);
	struct sockaddr_in si_other;
    int slen=sizeof(si_other);
    
	memset((char *) &si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(dest_port);
	if (inet_aton(dest_ip, &si_other.sin_addr)==0) {
		fprintf(stderr, "inet_aton() failed\n");
		return -1;
	}
	
	if (sendto(sd, buffer, bufsize, 0, (struct sockaddr *)&si_other, slen)==-1)
	perror("sendto()");


	/*printf("fd: %d \n", sdata->fd);
	int rc = sendto(sdata->fd, buffer, bufsize, 0, (struct sockaddr *)&sdata->addr, sizeof(struct sockaddr_in));
	if (rc < 0){
		if (errno != EWOULDBLOCK){
			perror("sendto_socket: sendto() failed");
			return -2;
		}
		return 0;
	}

	
	if (rc == 0){
		return -1;
	}
	
	return rc;*/
	return 0;
}

int get_send_socket(){
	int sd;
	
	sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sd < 0){
	  perror("socket failure");
	}
    
	return sd;
}

int getsocket(const char *source_ip, const char *source_port, int listen_port) {
	int listen_sd;
	int rc, retval;
	struct sockaddr_in addr;
	
	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof hints);
	hints.ai_family   = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	
	if ((retval = getaddrinfo(source_ip, source_port, &hints, &res)) != 0) {
		printf("getaddrinfo: \n");
		return -1;
	}

	/**
	* Create an AF_INET stream socket to receive incoming
	* connections on
	*/
	listen_sd = socket(res->ai_family, res->ai_socktype, 0);
	if (listen_sd < 0){
		perror("getsocket: socket() failed");
		return -1;
	}


	/**
	* Bind the socket
	*/
	memset(&addr, 0, sizeof(addr));
	addr.sin_family      = res->ai_family;
	addr.sin_addr.s_addr = htonl(INADDR_ANY); //INADDR_ANY
	addr.sin_port        = htons(listen_port);
	rc = bind(listen_sd, (struct sockaddr *)&addr, sizeof(addr));
	if (rc < 0){
		perror("getsocket: bind() failed");
		close(listen_sd);
		return -2;
	}

	return listen_sd;
}
