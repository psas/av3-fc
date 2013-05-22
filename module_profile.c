/*
 * module_profile.c
 *
 */
#include <sys/timerfd.h>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include "module_profile.h"
#include "fcfutils.h"

#define MAX_COUNT 10000000	//!< negative value indicates indefinite MAX_COUNT
static const int PROFILEMODE = 1;

static unsigned long int count = 0; //!< The number of times the loop has run.
static unsigned char buf[1024];
static int fd = -1;	//!< timer fd
static struct itimerspec t;
static struct timeval start;
static struct timeval end;


/**
 * The callback function for profiling.
 * Sent data is ignored, so we will fall through poll(2) next time around.
 * @fn profiling1_cb
 * @param None
 * @return None
 */
static void profiling1_cb (struct pollfd * pfd) {
	sendMessage_profile (NULL, 0);	//send messages
}

/**
 * The callback function for profiling.
 * Sent data is consumed and a new timer fd is set up.
 * @fn profiling2_cb
 * @param None
 * @return None
 */
static void profiling2_cb (struct pollfd * pfd) {
	int act_len = read (pfd->fd, buf, sizeof(buf));
	sendMessage_profile (buf, act_len);	//send messages
	timerfd_settime(fd, 0, &t, NULL);	//set up next timer
}

/**
 * Like profiling1_cb but sends more data.
 * Check asm code to see what really happens!
 */
static void profiling3_cb (struct pollfd * pfd) {
	int x = pfd->revents;
	sendMessage_profile3 (x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x);	//send messages
}


/**
 * Receive the message. If we've received MAX_COUNT, stop the loop.
 * @fm getMessage_profile
 * @param dummy params
 * @return
 */
void getMessage_profile(unsigned char *buf, int len) {
	count++;
	//printf("\nReceived %d out of %d messages.", count, MAX_COUNT);
	if (MAX_COUNT >= 0 && count == MAX_COUNT) {
		gettimeofday(&end, NULL);
		fcf_stop_main_loop();
	}
}


/**
 * Receive the message. If we've received MAX_COUNT, stop the loop.
 * Check asm code to see what really happens.
 * @fm getMessage_profile
 * @param a lot of dummy params
 * @return
 */
void getMessage_profile3(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j, int k, int l, int m, int n, int o, int p, int q, int r, int s, int t, int u, int v, int w, int x, int y, int z) {
	int value = a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z;
	getMessage_profile(NULL, value);	//increase count
}


/**
 * Initialize the profiling system.
 * @fn init_profiling
 * @param None
 * @return None
 */
void init_profiling() {
	gettimeofday(&start,NULL);
	end = start;

	pollfd_callback cb = NULL;
	t.it_interval.tv_sec = 0;
	t.it_interval.tv_nsec = 0;
	t.it_value.tv_sec = 0;
	t.it_value.tv_nsec = 1;
	fd = timerfd_create(CLOCK_MONOTONIC, 0);
	timerfd_settime(fd, 0, &t, NULL);
	switch (PROFILEMODE) {
	case 1:
		cb = profiling1_cb;
		break;
	case 2:
		cb = profiling2_cb;
		break;
	case 3:
		cb = profiling3_cb;
		for (int i = 0; i < 100; i++) {
			//add dummy fds that will be looped over in main loop
			//poll(2) ignores fds < 0
			fcf_add_fd (-1, POLLIN, cb);
		}
		break;
	default:
		printf ("\nno profiling\n");
		return;
	}
	fcf_add_fd (fd, POLLIN, cb);
	printf ("\nprofile fd : %d", fd);
}


void finalize_profiling() {
	if (timercmp (&start, &end, ==)) {
		//end not yet set; we are ending main loop prematurely
		gettimeofday(&end, NULL);
	}
	struct timeval diff;
	timersub(&end, &start, &diff);

	printf("\n\nFinished with count: %lu in %ld.%ld sec\n\n", count, diff.tv_sec, diff.tv_usec);

	if (fd >= 0) {
		fcf_remove_fd(fd);
		close(fd);
		fd = -1;
	}

}
