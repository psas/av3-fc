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
#include <error.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <linux/net_tstamp.h>
#include "utilities/utils_sockets.h"


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
		fprintf(stderr, "readsocketfromts: Connection closed\n");
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

int udp_socket(){
	int s = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, IPPROTO_UDP);
	if (s < 0){
		error(EXIT_FAILURE, errno, "udp_socket: socket() failed");
	}
	return s;
}

int bound_udp_socket(int port) {
	int s = udp_socket();

	struct sockaddr_in sin = {};
	sin.sin_family      = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port        = htons(port);

	if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0){
		perror("bound_udp_socket: bind() failed");
		close(s);
		exit(EXIT_FAILURE);
	}

	return s;
}

int timestamped_bound_udp_socket(int port) {
	int s = bound_udp_socket(port);

	int opts = SOF_TIMESTAMPING_RX_HARDWARE
	         | SOF_TIMESTAMPING_RX_SOFTWARE
	         | SOF_TIMESTAMPING_SYS_HARDWARE;
	if(setsockopt(s, SOL_SOCKET, SO_TIMESTAMPNS, &opts, sizeof(opts)) < 0){
		perror("timestamped_bound_udp_socket: setsockopt failed");
		close(s);
		exit(EXIT_FAILURE);
	}

	return s;
}
