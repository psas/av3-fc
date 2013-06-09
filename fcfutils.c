/*
  PSAS Flight Control Framework
  Copyright (C) 2013  
  Team Elderberry [Ron Astin, Clark Wachsmuth, Chris Glasser, Josef Mihalits, Jordan Hewitt, Michael Hooper]

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <libusb-1.0/libusb.h>
#include <signal.h>
#include "fcfutils.h"
#include "fcfmain.h"

#define FDS_INIT_SIZE 1
#define FDS_EXPANSION_FACTOR 2


/*	
 *	This struct holds the callback functions and souce tokens of the devices.
 */
struct fcffd{
  pollfd_callback callback;
  char cb_cat;
};

static const char STANDARD = 0;	//< Standard callback
static const char PPC = 1;	//< Per poll cycle callback

static struct pollfd * fds = NULL;	//< File descriptor array
static struct fcffd  * fdx = NULL;	//< File description array
static int nfds;		//< Number of file descriptors in arrays
static int fd_array_size;	//< Allocated size of file descriptor array, fds
static int run_fc;		//< Main loop is running true/false

extern void fcf_initialize(void);
extern void fcf_finalize (void);

/*
 * Initialization for fcf data structures
 */
static int init_fcf(){
  fd_array_size = FDS_INIT_SIZE;

  //initializing both file descriptor arrays
  fds = (struct pollfd *) malloc(fd_array_size * sizeof(struct pollfd));
  fdx = (struct fcffd *) malloc(fd_array_size * sizeof(struct fcffd));
  
  if(!fds){
	  fprintf(stderr, "Could not allocate memory for file descriptor array.");
	  return -1;
  }
  else if(!fdx){
	  fprintf(stderr, "Could not allocate memory for callback array.");
	  return -1;
  }
	
  nfds = 0;  //< Number of file descriptors in array
  run_fc = 0;  //< Main loop is running True/False

  return 0;
}


/*
 * Deallocate fcf data structures
 */
static void finalize_fcf(){
	free(fds);
	free(fdx);
	nfds = -1;
}


/*
 *	Increases size of the file desciptor and file description arrays
 */
static int expand_arrays(){
  struct pollfd * fds_temp;
  struct fcffd * fdx_temp;

  fd_array_size *= FDS_EXPANSION_FACTOR; // Expand array by pre-defined factor

  // Increase size of fds array
  fds_temp = realloc(fds, fd_array_size * sizeof(struct pollfd));
  if(fds_temp == NULL){
    return -1;  //if failed
  }
  fds = fds_temp;
	
  // Increase size of fdx array
  fdx_temp = realloc(fdx, fd_array_size * sizeof(struct fcffd));
  if(fdx_temp == NULL){
    return -1;  // if failed
  }
  fdx = fdx_temp;

  return 0;
}

/*
 *    Add file descriptor into both fdx and fds arrays
 */ 
int fcf_add_fd(int fd, short events, pollfd_callback cb){
  // Checks to see if fd arrays are full, if they are expand arrays.
  if(fd_array_size == nfds){
    expand_arrays();
  }
  
  // Filling file descriptor arrays with fd and callback data
  fds[nfds].fd = fd;
  fds[nfds].events = events;
  fdx[nfds].callback = cb;
  fdx[nfds].cb_cat = STANDARD;
  nfds++;

  return nfds - 1; // return value is the index of the newest file descriptor
}


/*
 *    Per poll loop file descriptor add
 */
int fcf_add_fd_ppc(int fd, short events, pollfd_callback cb){
  int i = fcf_add_fd (fd, events, cb);
  if(i >= 0){
    fdx[i].cb_cat = PPC;
  }
  return i;
}

/*
 *    Removes the indicated file descriptor from the arrays.
 */
void fcf_remove_fd(int fd){
 
  // If there are no fds, return
  if(nfds <= 0)
    return;

  for(int i = 0; i < nfds; i++){
    if(fds[i].fd == fd && i == (nfds - 1)){
      nfds--;
    }
    else if(fds[i].fd == fd){
      memmove(&fds[i], &fds[nfds - 1], sizeof(struct pollfd));
      memmove(&fdx[i], &fdx[nfds - 1], sizeof(struct fcffd));
      nfds--;
    }
  }
}

/*
 *    Function returns file descriptor information for specified array index
 */
struct pollfd *fcf_get_fd(int idx){
  return &fds[idx];
}

/*
 *    Stops main poll loop from running
 */
void fcf_stop_main_loop(){
  run_fc = 0;
}

/*
 *    Starts main poll loop running
 */
static void fcf_start_main_loop(){
  run_fc = 1;
}

/*
 *    Detects change in file descriptors and polls fd arrays
 */
static int fcf_run_poll_loop(){
  //currently returns -1 on error; 0 on success.
  pollfd_callback ppc[nfds];
  int nppc = 0, ret = 0;
  //int count = 0;

  fcf_start_main_loop();
  while(run_fc){
    fflush(stdout);

    errno = 0;
    int rc = poll(fds, nfds, -1);

    switch (rc){
    case -1: // error
      if(errno != EINTR){
	perror("run_main_loop: poll returned with error");
	ret = -1;
	fcf_stop_main_loop();
      }
      break;
    case 0: // timeout
      break;
    default:
      nppc = 0;
      for(int i = 0; i < nfds && rc > 0; i++){
	if(fds[i].revents != 0){
	  rc--;
	  if(fdx[i].cb_cat == STANDARD){
	    // callback for this active fd is a standard callback
	    fdx[i].callback(&fds[i]);
	  } 
	  else{
	    // callback for this active fd is a "per poll cycle" callback
	    int j;
	    for(j = 0; j < nppc && ppc[j] != fdx[i].callback; j++)
	      { /*empty*/ }
	    if(j == nppc){
	      // a new ppc callback
	      // add to ppc so that callback will be called at end of poll cycle
	      ppc[nppc++] = fdx[i].callback;
	    } 
	    else{
	      //we have seen this callback before
	      //printf("\n multiple active ppc, ignoring callback for fd[%d]: fd=%d", i, fds[i].fd);
	    }
	  }
	} // (revents set)
      } // (for i)

      // handle ppc callbacks
      for(int j = 0; j < nppc; j++){
	// if callback wants to access the fds, callback
	// is expected to know the indices into the fds array
	// i.e., module must store return values it gets from fcf_addfdPpc
	//printf("\n calling ppc callback [%d]", j);
	ppc[j](fds);
      }

      break;
    }

  }

  //printf("\n exiting main loop");
  return ret;
}


static void signalhandler(int signum){
  if(signum == SIGINT){
    fcf_stop_main_loop ();
  }
}


int main(int argc, char *argv[]){
  printf("\n FLIGHT CONTROL FRAMEWORK V1.0 -- Copyright (C) 2013\n"
	 " : Ron Astin, Clark Wachsmuth, Chris Glasser\n : Josef Mihalits, Jordan Hewitt, Michael Hooper\n\n"
	 "\n\n\n");
  signal(SIGINT, signalhandler);

  int rc = init_fcf();			//< FCF init that sets up fd structures
  if(rc == 0){
	  fcf_initialize();			//< fcfmain init function for user modules
	  int rc = fcf_run_poll_loop();
	  fcf_finalize();			//< fcfmain finalize function for user modules
	  finalize_fcf();			//< FCF finalize that deallocates fd structures
	  if(rc == 0) {
		return EXIT_SUCCESS;
	  }
  }
  return EXIT_FAILURE;
}
