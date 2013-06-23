/*
 * ethmux.c
 *
 */

#include "utils_sockets.h"
#include "fcfutils.h"
#include "net_addrs.h"

static unsigned char buffer[1024];

void ethmux_cb(struct pollfd *pfd){
	int rc = readsocket(pfd->fd, buffer, sizeof(buffer));
}

int ethmux_init(){
	int fd = getsocket(ADIS_IP, ADIS_PORT_S, FC_LISTEN_PORT);
	int rc = fcf_add_fd(fd, POLLIN, ethmux_cb);
	return rc;
}

void ethmux_final(){

}
