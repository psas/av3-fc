/* dumb flight computer
 *
 * Read from a variety of devices:
 * 1. several GFEs:  Power, IMU 1, IMU 2
 * 2. GPS, USB serial device
 * 3. a fifo written to by launch control logged in via ssh to assert rocket ready
 *
 * All data read is dumped in two places
 * 1. network connection to ground (telemetry)
 * 2. log file
 */

/* Possibly easiest is to start from the old fcfifo and ltc-fc-common projects */
/*  fcfifo/net.c */
/*	ltc-fc-common/net*.c */

#include <stdio.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>

FILE *log;

struct sockaddr_in remote = { AF_INET };


#define UDP_SIZE 1420-4
struct {
	unsigned int seq;
	char buf[UDP_SIZE];
} bucket = { 0 };
char *pos = bucket.buf;

int net_write(char *buf, int len)
{
	if (pos - bucket.buf + len > UDP_SIZE)
	{
		sendmsg(net_fd, &bucket, pos - bucket + 4, 0);
		pos = bucket.buf;
		++bucket.seq;
	}
	memcpy(pos, buf, len);
	pos += len;

	fwrite(log, len, 1, buf);
}

int main(int argc, char *argc[])
{
	log = fopen("log", "w");
	if (log == NULL)
		exit(1);
		
	// init UDP socket
	remote.sin_port = htons(REMOTE_PORT);
	remote.sin_addr.s_addr = htonl(REMOTE_ADDR);

	// initialize devices
	const int num_fds = 0;
	struct pollfd fds[num_fds];

	while (1)
	{
		int i, n = poll(fds, num_fds, -1);
		if(n < 0 && errno != EINTR)
		{
			fprintf(log, "\npoll: %s\n", strerror(errno));
		}
		for(i = 0; n > 0; --n)
		{
			while(!fds[i].revents)
				++i;
			if(fds[i].revents & POLLIN)
				handler[i]->read(fds[i].fd);
			if(!(fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)))
				continue;
					
		}
	}
	return 0;
}
