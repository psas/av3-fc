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
#include <time.h>
#include <linux/net_tstamp.h>


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

int readsocketfrom(int fd, unsigned char *buffer, int bufsize, struct sockaddr *sender, socklen_t *addrlen) {
	/**
	* Receive data packet on this socket,
	* and also return the sender's address.
	*/
	int rc = recvfrom(fd, buffer, bufsize, 0, sender, addrlen);
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
	
	return rc;
}


int readsocketfromts(int fd, unsigned char *buffer, int bufsize, struct sockaddr_in *sender, socklen_t addrlen, struct timespec *ts){
	struct msghdr msg;
	struct cmsghdr *cmsg;
	struct iovec entry;
	struct {
	struct cmsghdr cm;
		char control[512];
	} control;

	memset(&msg, 0, sizeof(msg));
	msg.msg_iov = &entry;
	msg.msg_iovlen = 1;
	entry.iov_base = buffer;
	entry.iov_len = bufsize;
	msg.msg_name = (caddr_t)sender;
	msg.msg_namelen = addrlen;
	msg.msg_control = &control;
	msg.msg_controllen = sizeof(control);

	int rc = recvmsg(fd, &msg, 0);
	if (rc < 0){
		if (errno != EWOULDBLOCK){
			perror("readsocketfromts: recvmsg() failed");
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

	/** Extract info from msg **/
	//timestamp info
	for (cmsg = CMSG_FIRSTHDR(&msg); cmsg; cmsg = CMSG_NXTHDR(&msg, cmsg)){
		if(cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SO_TIMESTAMPNS){
			ts->tv_nsec = ((struct timespec *)CMSG_DATA(cmsg))->tv_nsec;
			ts->tv_sec = ((struct timespec *)CMSG_DATA(cmsg))->tv_sec;
		}
	}
	//data
	if(msg.msg_iovlen > 1){
		printf("utils_sockets rsft: iovlen greater than 1\n");
	}

	return rc;
}


int sendto_socket(int sd, char *buffer, int bufsize, const char *dest_ip, int dest_port) {
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

int get_send_from_socket(int send_port) {
	struct sockaddr_in sin = {};
	int sock;

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(send_port);

	bind(sock, (struct sockaddr *)&sin, sizeof(sin));
	/* Now bound, get the address */
	//slen = sizeof(sin);
	//getsockname(sock, (struct sockaddr *)&sin, &slen);
	//port = ntohs(sin.sin_port);

	return sock;
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
	 * Turn on timestamping
	 */
	int opts = SOF_TIMESTAMPING_RX_HARDWARE | SOF_TIMESTAMPING_RX_SOFTWARE |
			SOF_TIMESTAMPING_SYS_HARDWARE;
	setsockopt(listen_sd, SOL_SOCKET, SO_TIMESTAMPNS, &opts, sizeof(opts));

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
