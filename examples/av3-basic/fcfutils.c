/*
 * fcfutils.c
 *
 */
#include <poll.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "fcfutils.h"

#define FCF_POLLSIZE 100

static const char standard = 0;	///< standard callback
static const char ppc = 1;		///< per poll cycle callback

static struct pollfd fds[FCF_POLLSIZE];			///< first param to poll(2)
static int nfds = 0;							///< second param to poll(2)
static pollCallback callbacks[FCF_POLLSIZE];	///< callbacks parallel to fds array
static char cbCat[FCF_POLLSIZE];				///< the type (i.e. category) of the callback (standard, ppc)

static int run_fc = 0;


static void debug_fd (const char *msg, int i, struct pollfd *pfd);



//add standard file descriptor
//return index in fds; i.e. value of >= 0 is ok; -1 is error
int fcf_addfd (int fd, short events, pollCallback cb) {
	if (nfds >=  FCF_POLLSIZE) {
		return -1;
	}
	unsigned int i = nfds++;
	fds[i].fd = fd;
	fds[i].events = events;
	callbacks[i] = cb;
	cbCat[i] = standard;
	return i;
}

//add per poll cycle file descriptor
//return index in fds; i.e. value of >= 0 is ok; -1 is error
int fcf_addfd_ppc (int fd, short events, pollCallback cb)
{
	int i = fcf_addfd (fd, events, cb);
	if (i >= 0) {
		cbCat[i] = ppc;
	}
	return i;
}

//remove file descriptor
int fcf_removefd (int fd) {
	for (int i = 0; i < nfds; i++) {
		if (fds[i].fd == fd) {
			//we found fd we want to remove
			nfds--;
			if (i < nfds) {
				//remove fd at index i by overwriting it with last fd
				fds[i] = fds[nfds];
				callbacks[i] = callbacks[nfds];
				cbCat[i] = cbCat[nfds];
			}
			return 0;
		}
	}
	return -1; //fd was not in array
}


void stop_main_loop() {
	run_fc = 0;
}

static void start_main_loop() {
	run_fc = 1;
}

int run_main_loop() {
	pollCallback ppc[FCF_POLLSIZE];
	int nppc= 0;
	int count = 0;
	int ret = 0;

	start_main_loop();
	while (run_fc) {

		for (int i = 0; i < nfds; i++) {
			debug_fd("\nbefore poll<<< ", i, &fds[i]);
		}
		printf("\nwaiting");
		fflush (stdout);

		errno = 0;
		int rc = poll(fds, nfds, -1);
		printf ("\n%d. poll returned with rc=%d errno=%d", count++, rc, errno);
		for (int i = 0; i < nfds; i++) {
			debug_fd("\n   after poll>>> ", i, &fds[i]);
		}

		switch (rc) {
		case -1: //error
			if (errno != EINTR) {
				perror ("run_main_loop: poll returned with error");
				ret = -1;
				stop_main_loop();
			}
			break;
		case 0: //timeout
			printf("poll timed out");
			break;
		default:
			nppc = 0;
			for (int i = 0; i < nfds && rc > 0; i++) {
				if(fds[i].revents != 0) {
					debug_fd("\n active fd ", i, &fds[i]);
					rc--;
					if (cbCat[i] == standard) {
						//callback for this active fd is a standard callback
						callbacks[i](&fds[i]);
					} else {
						//callback for this active fd is a "per poll cycle" callback
						int j;
						for (j = 0; j < nppc && ppc[j] != callbacks[i]; j++)
						{ /*empty*/ }
						if (j == nppc) {
							//a new ppc callback
							//add to ppc so that callback will be called at end of poll cycle
							ppc[nppc++] = callbacks[i];
						} else {
							//we have seen this callback before
							printf ("\n multiple active ppc, ignoring callback for fd[%d]: fd=%d", i, fds[i].fd);
						}
					}
				} // (revents set)
			} // (for i)

			//handle ppc callbacks
			for (int j = 0; j < nppc; j++) {
				//if callback wants to access the fds, callback
				//is expected to know the indices into the fds array
				//i.e., module must store return values it gets from fcf_addfdPpc
				printf ("\n calling ppc callback [%d]", j);
				ppc[j](fds);
			}

			break;
		}

	}

	printf("\n exiting main loop");
	return ret;
}

static void debug_fd (const char *msg, int i, struct pollfd *pfd) {
	printf("%s fd[%d]: fd=%d events=%X revents=%X", msg, i, pfd->fd, pfd->events, pfd->revents);
	int re = pfd->revents;
	if (re & POLLERR) printf("\nPOLLERR - Error condition");
	if (re & POLLHUP) printf("\nPOLLHUP - Hang up");
	if (re & POLLNVAL) printf("\nPOLLNVAL - Invalid request: fd not open");
	fflush (stdout);
}
