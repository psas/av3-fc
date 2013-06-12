/*
 * module_adis_sim.c
 *
 */
#include <sys/timerfd.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include "module_adis_sim.h"
#include "fcfutils.h"
#include "utils_sockets.h"
#include "net_addrs.h"
#include "psas_packet.h"

#define FILENAME "sim_data/adis16405_log.txt-8June2013.csv"

static int fd = -1;	//!< timer fd
static struct itimerspec t;
static int sd; // socket descriptor

FILE* stream = NULL;


// timestamp,ax,ay,az,gx,gy,gz,mx,my,mz,C*/

ADIS_packet * prepare_packet(char *line){
	static ADIS_packet ap;
	const char* tok;
	char pdata[1024];
	int count = 1;
	int pdata_size = 0;

    tok = strtok(line, ",");
	while(tok != NULL){
        switch(count){
			case 1:
				memcpy(ap.ID, tok, 4);
			
				break;
			case 2:
				memcpy(ap.timestamp, tok, 6);
			printf("%s: \n", tok);
				break;
			case 3: 
				memcpy(&ap.data_length, tok, 2);
				break;
			default:
				if(pdata_size < sizeof(ADIS16405_burst_data)){
					//unsigned int val = atoi(tok);
					//printf("%d: \n", val);
					//uint16_t value = 0;
					//sprintf(tok, "%u", value);
					memcpy(&pdata[pdata_size], tok, 2);
					pdata_size += 2;
				}
				break;
		}
		memcpy(&ap.data, pdata, pdata_size);
		tok = strtok(NULL, ",");
		count++;
    }
	return &ap;
}


static void adis_sim_cb (struct pollfd * pfd) {
	static int count = 0;
	char line[1024];

	
	if(fgets(line, 1024, stream) != NULL){
		char* tmp = strdup(line);
		ADIS_packet * aptemp = prepare_packet(tmp);
		if(aptemp != NULL){
			memcpy(line, aptemp, sizeof(ADIS_packet));
			sendto_socket(sd, line, sizeof(ADIS_packet), FC_IP, FC_LISTEN_PORT);
		}
		free(tmp);
	}

	timerfd_settime(fd, 0, &t, NULL);	//set up next timer
	count++;
	count = count % 31;
}


void init_profiling() {
	pollfd_callback cb = NULL;
	t.it_interval.tv_sec = 0;
	t.it_interval.tv_nsec = 0;
	t.it_value.tv_sec = 0;
	t.it_value.tv_nsec = 300000;
	fd = timerfd_create(CLOCK_MONOTONIC, 0);
	timerfd_settime(fd, 0, &t, NULL);
	
	cb = adis_sim_cb;
	sd = get_send_socket();
		
	fcf_add_fd (fd, POLLIN, cb);

	char line[1024];
	char *rc;
	stream = fopen(FILENAME, "r");
	if(fgets(line, 1024, stream));
	if(fgets(line, 1024, stream));
	if(fgets(line, 1024, stream));
	
}


void finalize_profiling() {

}
