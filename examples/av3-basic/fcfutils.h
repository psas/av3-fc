/*
 * fcfutils.h
 *
 *  Created on: Feb 15, 2013
 */

#ifndef FCFUTILS_H_
#define FCFUTILS_H_

#include <poll.h>

typedef int (*pollCallback)(struct pollfd *pfd);
int fcf_addfd (int fd, short events, pollCallback cb);
int fcf_addfd_ppc (int fd, short events, pollCallback cb);
int fcf_removefd (int fd);
int run_main_loop (void);
void stop_main_loop (void);

#endif /* FCFUTILS_H_ */
