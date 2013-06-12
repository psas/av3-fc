/**
 *  @file logger.c
 */

#include <stdio.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include "adis.h"
#include "logger.h"
#include "utils_sockets.h"
#include "net_addrs.h"

#define P_LIMIT 1500

static FILE *fp = NULL;
static char filename[50];
static char log_buffer[1500]; 		// Global so destructor can flush final data
static int log_buffer_size = 0;
static int sd;


void logger_init() {
	sprintf(filename, "logfile-%d.log", (int)time(NULL));
	fp = fopen(filename, "w+");
	if(!fp){
		fprintf (stderr, "disk logger: could not open file %s for writing: %s\n", filename, strerror(errno));
	}
	setbuf(fp, NULL);
	sd = get_send_socket();
	printf("Filling packet: ");
}


void logger_final() {
	if(log_buffer_size > 0){
		fwrite(log_buffer, sizeof(char), log_buffer_size, fp);
	}
	fclose(fp);
}


void  log_getPositionData_adis(unsigned char *data, int size) {
	if(log_buffer_size + size >= P_LIMIT){
		// Send current buffer to disk/wifi
		printf("\nDumping packet to disk and wifi.\n\n");
		fwrite(log_buffer, sizeof(char), log_buffer_size, fp);
		sendto_socket(sd, log_buffer, log_buffer_size, WIFI_IP, WIFI_PORT);
		log_buffer_size = 0;
		printf("Filling packet: ");
	}

	memcpy(&log_buffer[log_buffer_size], data, size);
	log_buffer_size += size;
	memcpy(&log_buffer[log_buffer_size], "\n", 1);
	log_buffer_size++;
	printf("-");
}
