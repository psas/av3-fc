/* starting point for the socket code was:
 * http://publib.boulder.ibm.com/infocenter/iseries/v6r1m0/index.jsp?topic=/rzab6/poll.htm
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
#include "netutils.h"

int readsocket(int fd, unsigned char *buffer, int bufsize) {
	/*****************************************************/
	/* Receive data on this connection until the         */
	/* recv fails with EWOULDBLOCK. If any other        */
	/* failure occurs, we will close the                 */
	/* connection.                                       */
	/*****************************************************/
	int rc = recv(fd, buffer, bufsize, 0);
	if (rc < 0)
	{
		if (errno != EWOULDBLOCK)
		{
			perror("  recv() failed");
			return -1;
		}
		return 0;
	}

	/*****************************************************/
	/* Check to see if the connection has been           */
	/* closed by the client                              */
	/*****************************************************/
	if (rc == 0)
	{
		printf("  Connection closed\n");
		return -1;
	}

	/*****************************************************/
	/* Data was received                                 */
	/*****************************************************/
	printf("  %d bytes received\n", rc);

	return rc;
}

int getsocket(int serverport) {
	int listen_sd;
	int rc;
	//int on = 1;
	struct sockaddr_in addr;

	/*************************************************************/
	/* Create an AF_INET stream socket to receive incoming       */
	/* connections on                                            */
	/*************************************************************/
	listen_sd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sd < 0)
	{
		perror("socket() failed");
		exit(EXIT_FAILURE);
	}

//	/*************************************************************/
//	/* Allow socket descriptor to be reuseable                   */
//	/*************************************************************/
//	rc = setsockopt(listen_sd, SOL_SOCKET,  SO_REUSEADDR, (char *)&on, sizeof(on));
//	if (rc < 0)
//	{
//		perror("setsockopt() failed");
//		close(listen_sd);
//		exit(-1);
//	}

	/*************************************************************/
	/* Set socket to be nonblocking. All of the sockets for    */
	/* the incoming connections will also be nonblocking since  */
	/* they will inherit that state from the listening socket.   */
	/*************************************************************/
//	rc = ioctl(listen_sd, FIONBIO, (char *)&on);
//	if (rc < 0)
//	{
//		perror("ioctl() failed");
//		close(listen_sd);
//		exit(EXIT_FAILURE);
//	}

	/*************************************************************/
	/* Bind the socket                                           */
	/*************************************************************/
	memset(&addr, 0, sizeof(addr));
	addr.sin_family      = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); //INADDR_ANY
	addr.sin_port        = htons(serverport);
	rc = bind(listen_sd, (struct sockaddr *)&addr, sizeof(addr));
	if (rc < 0)
	{
		perror("bind() failed");
		close(listen_sd);
		exit(EXIT_FAILURE);
	}

	/*************************************************************/
	/* Set the listen back log                                   */
	/*************************************************************/
	rc = listen(listen_sd, 1);
	if (rc < 0)
	{
		perror("listen() failed");
		close(listen_sd);
		exit(EXIT_FAILURE);
	}

	/*****************************************************/
	/* Accept each incoming connection. If               */
	/* accept fails with EWOULDBLOCK, then we            */
	/* have accepted all of them. Any other              */
	/* failure on accept will cause us to end the        */
	/* server.                                           */
	/*****************************************************/
	int new_sd = accept(listen_sd, NULL, NULL);
	if (new_sd < 0)
	{
		if (errno != EWOULDBLOCK)
		{
			perror("  accept() failed");
			exit (EXIT_FAILURE);
		}
	}

	return new_sd;
}
