/**
 * @file fcfutils.h
 * @brief Utility functions for the flight control framework (fcf)
 * @details The core framework file. It starts the main function, introducing the program. This program has signal handlers for when the user exits abruptly using ctrl+C, allowing a graceful shutdown of the software. The main function also initializes the framework and the modules before running the main loop.

<<<<<<<<<<<<<<<<<<<< LICENSING INFORMATION >>>>>>>>>>>>>>>>>>>>
\n FLIGHT CONTROL FRAMEWORK V0.1  Copyright (C) 2013\n
Ron Astin, Clark Wachsmuth, Chris Glasser, Josef Mihalits, Jordan Hewitt, Michael Hooper]\n\n
----------------------------------------------------------------\n
This program comes with ABSOLUTELY NO WARRANTY;\n for details please
visit http://www.gnu.org/licenses/gpl.html.\n\n
This is free software, and you are welcome to redistribute it\n
under certain conditions; For details, please visit\n
http://www.gnu.org/licenses/gpl.html.\n
----------------------------------------------------------------\n\n\n

                                           
 * @author Team Elderberry
 * @date March 2nd, 2013
 */
#ifndef _FCFUTILS_
#define _FCFUTILS_

#define TRUE             1
#define FALSE            0

#include <poll.h>

/**
 * @brief pollfd callback function pointer 
 * @details takes in a pollfd pointer and acts on individual callback functions.
 */
typedef void (*pollfd_callback)(struct pollfd *);
/**
 * @brief adds file descriptor on to the end of the array
 * @details Checks to see if the file descriptor arrays are full. If the arrays are full, it calls the expand_arrays() function (this will double the size of the arrays). It adds information to two arrays, the fds and fdx arrays.  The fds array has pollfd pointers (required by the poll system call) and the fdx array has fcffd pointers (required by our framework [containing callback functions and other information])
 * @param fd - integer correlating to the file descriptor from the process's file descriptor table
 * @param events - flags for which revents should change
 * @param cb - poll callback function for the file descriptor
 * @return index value of newest file descriptor
 */
extern int fcf_add_fd(int fd, short events, pollfd_callback cb);
/**
 * @brief simply removes a specified file descriptor from the arrays
 * @details If the fd is in the arrays, the fd is removed from both the fds and fdx arrays.
 * @param fd - integer correlating to the file descriptor from the process's file descriptor table
 */
extern void fcf_remove_fd(int fd);

/**
 * @brief returns the fds array info
 * @details Specifically this function returns the pollfd* information from the fds array for the specified index value
 * @param idx - index value
 * @return pollfd* info for specified index value
 */
extern struct pollfd * fcf_get_fd(int idx);
/**
 * @brief stops main loop
 */
extern void fcf_stop_main_loop(void);

/**
 * @brief Main function for framework
 * @details Prints the licensing information, and software version number.  It contains the signal handler for graceful shutdown should the user CTRL-C out of the program. It then initializes the framework and runs the polling loop.
 * @return EXIT_SUCCESS
 * @return EXIT_FAILURE
 */
int main(int argc, char *argv[]);

#endif
