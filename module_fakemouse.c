
/******************************************************************************
*
*	VIRTUAL DEVICE USER MODULE TEMPLATE
*
*	This template was created to help speed up the process of building user
*	modules that use sockets. To start using this file, follow these steps:
*
*
*		STEP 1.	Search and Replace ###DEVTAG### with the token you've assigned 
*				to your device (e.g. gyro) in both this file and the 
*				accompanying header file. This step is to ensure that
*				there are no namespace conflicts.
*
*		STEP 2.	Make sure the callback in the data_callback() function is 
*				setup in fcfmain.c and fcfmain.h. Also, make sure the
*				arguments to this callback are sufficient to the needs of
*				the device.
*
*		STEP 3.	Rename both this file and the accompanying header file to
*				a unique filename that includes the token. For the sake of
*				convention, we recommend module_virtdev_{token}.c/.h.
*
**/


#include <stdio.h>
#include <unistd.h>

#include "utils_sockets.h"
#include "fcfutils.h"
#include "fcfmain.h"

static unsigned char buffer[1000];
static int fd = -1;

void getMessage_fakemouse(unsigned char *buf, int len) {
	//printf (".");
}


//active fd
static void virtFAKEMOUSE_cb (struct pollfd *pfd) {
	int rc = readsocket(pfd->fd, buffer, sizeof(buffer));
	if (rc > 0) {
		sendMessage_fakemouse(buffer, rc);
	}
}

static int initvirtdev (const char* devname, int port, pollfd_callback cb) {
	printf ("probing %s: (waiting for connection 127.0.0.1:%d)\n", devname, port);
	fd = getsocket(port);
	printf ("fd = %d\n", fd);
	int rc = fcf_add_fd(fd, POLLIN, cb);
	return rc;
}


void init_fakemouse() {
	initvirtdev ("virt_FAKEMOUSE", 9876, virtFAKEMOUSE_cb);
}


void finalize_fakemouse() {
	fcf_remove_fd (fd);
	if (fd >= 0) {
		close (fd);
	}
}
