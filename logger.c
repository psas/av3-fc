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

#define P_LIMIT 1500
#define WIFI_IP "192.168.0.20"
#define WIFI_PORT 36000

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
		fwrite(log_buffer, sizeof(char), log_buffer_size, fp);
		sendto_socket(sd, log_buffer, log_buffer_size, WIFI_IP, WIFI_PORT);
		log_buffer_size = 0;
	}

	memcpy(&log_buffer[log_buffer_size], data, size);
	log_buffer_size += size;
	memcpy(&log_buffer[log_buffer_size], "\n", 1);
	log_buffer_size++;
	printf("size: %d (+ %d) \n", log_buffer_size, size);
}
