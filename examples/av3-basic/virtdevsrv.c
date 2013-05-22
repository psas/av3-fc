#include <stdio.h>
#include "netutils.h"
#include "fcfutils.h"
#include "virtdevsrv.h"
#include "miml.h"


static unsigned char buffer[1000];


static int common_cb (uint32_t fourcc, int fd) {
//	printf_tagged_message(fourcc, "\n common_cb");
//	flush_buffers();

	int rc = readsocket(fd, buffer, sizeof(buffer));
	if (rc > 0) {
//		printbuffer(fourcc, buffer, rc);	//print to screen
//		write_tagged_message(fourcc, buffer, rc);
//		flush_buffers();
		FCF_Log(fourcc, buffer, rc);
	}
	return rc;
}

//active fd
static int virtgyro_cb (struct pollfd *pfd) {
	return common_cb (FOURCC('G', 'Y', 'R', 'O'), pfd->fd);
}

static int virtacc_cb (struct pollfd *pfd) {
	return common_cb (FOURCC('A', 'C', 'C', 'O'), pfd->fd);
}


static int initvirtdev (const char* devname, int port, pollCallback cb) {
	printf ("probing %s: (waiting for connection localhost:%d)\n", devname, port);
	int fd = getsocket(port);
	int rc = fcf_addfd (fd, POLLIN, cb);
	return rc;
}



//instead of having multiple callback functions,
//have one and pass through parameters?

int init_virtgyro() {
	return initvirtdev ("virt gyro", 8081, virtgyro_cb);
}

int init_virtacc() {
	return initvirtdev ("virt acc", 8082, virtacc_cb);
}

