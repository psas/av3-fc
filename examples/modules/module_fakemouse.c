
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

#include "utils_sockets.h"
#include "fcfutils.h"
#include "fcfmain.h"

static unsigned char buffer[1000];


static void common_cb(const char * src, int fd){
	int rc = readsocket(fd, buffer, sizeof(buffer));
	if (rc > 0) {
		fcf_callback_virtdev(src, buffer, rc);
		printf("RC > 0");
	}
}

//active fd
static void virtFAKEMOUSE_cb (struct pollfd *pfd) {
	common_cb("virt_FAKEMOUSE", pfd->fd);
}

static int initvirtdev (const char* devname, int port, pollfd_callback cb) {
	printf ("probing %s: (waiting for connection localhost:%d)\n", devname, port);
	int fd = getsocket(port);
	int rc = fcf_add_fd(fd, POLLIN, cb);
	return rc;
}

//instead of having multiple callback functions,
//have one and pass through parameters?

void init_virtFAKEMOUSE() {
	initvirtdev ("virt_FAKEMOUSE", 9876, virtFAKEMOUSE_cb);
}


