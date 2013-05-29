
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
*				a unique filename in the root framework directory that includes
*				the token. For the sake of convention, we recommend 
*				module_virt{token}.c/.h.
*
*		STEP 4. Create the MIML file for the device by using the header2Miml.py
*				script. Example: ./header2Miml.py module_virt(token)
*
**/


#include <stdio.h>

#include "utils_sockets.h"
#include "fcfutils.h"
#include "fcfmain.h"
#include "psas_packet.h"

static unsigned char buffer[1000];
static          ADIS_packet          adis16405_udp_data;

static void common_cb(const char * src, int fd){
	int rc = readsocket(fd, buffer, sizeof(buffer));
	if (rc > 0) {
	        memcpy (&adis16405_udp_data, buffer, sizeof(ADIS_packet));
		log_getPositionData_adis(&adis16405_udp_data);
	}
}

//active fd
static void virtADIS_cb(struct pollfd *pfd){
	common_cb("virt_ADIS", pfd->fd);
}

static int initvirtdev(const char* devname, int port, pollfd_callback cb) {
	printf ("probing %s: (waiting for connection localhost:%d)\n", devname, port);
	int fd = getsocket(port);
	int rc = fcf_add_fd(fd, POLLIN, cb);
	return rc;
}

//instead of having multiple callback functions,
//have one and pass through parameters?

void adis_init(){
	initvirtdev("virt_ADIS", 36000, virtADIS_cb);
}

void adis_final(){
}


