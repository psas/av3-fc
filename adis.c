


#include <stdio.h>
#include <string.h>

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
		sendADISData(&adis16405_udp_data);
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


