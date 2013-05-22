/**
 * profile.c
 *
 *  Created on: Feb 11, 2013
 *      Author: jordan
 */
#include <sys/timerfd.h>
#include <stdio.h>
#include <unistd.h>
#include "profile.h"
#include "miml.h"
#include "fcfutils.h"

#define MAX_COUNT 10000000

static int count = 0; //!< The number of times the loop has run.

//static unsigned char buf[1024];
static int thefd;
static struct itimerspec t;


/**
 * The callback function for profiling.
 * @fn profiling_cb
 * @param None
 * @return None
 *
 */
static int profiling_cb (struct pollfd *pfd) {
	//int act_len = read (pfd->fd, buf, sizeof(buf));
	//printf ("\n timerfd callback: reading %d", (int) act_len);
	FCF_ProfSendMsg("");
	//timerfd_settime(thefd, 0, &t, NULL);
	return 0; //act_len;
}


/**
 * Initialize the profiling system.
 * @fn InitProfiling
 * @param None
 * @return None
 *
 */
void InitProfiling() {

	t.it_interval.tv_sec = 0;
	t.it_interval.tv_nsec = 0;
	t.it_value.tv_sec = 0;
	t.it_value.tv_nsec = 1;
    thefd = timerfd_create(CLOCK_REALTIME, 0);
    timerfd_settime(thefd, 0, &t, NULL);
    fcf_addfd(thefd, POLLIN, profiling_cb);
    printf ("\nprofile fd : %d", thefd);
}


/**
 * Recieve the message. if we've recieved MAX_COUNT, stop the loop.
 * @fm profReceiveMsg
 * @param int count
 * @return
 *
 */
void profReceiveMsg (const char *msg) {// user receives message here
   // printf("\nReceived %d out of %d messages.", count, MAX_COUNT);
   // fflush (stdout);
    if (count == MAX_COUNT){
        stop_main_loop();
    }
    count++;
}
