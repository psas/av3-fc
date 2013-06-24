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

#ifndef FCF_FAKE_ADIS_DATA_FILE
#define FCF_FAKE_ADIS_DATA_FILE  "mock_devices/test.ADIS.bin"
#endif

static int fd = -1;	//!< timer fd
static struct itimerspec t;
static int sd; // socket descriptor

FILE* stream = NULL;

// ADIS has 12 uint16_t's
#define ADIS_DATA_LEN 12
static uint16_t values_unpack[ADIS_DATA_LEN];
static char cr;

static void adis_sim_cb (struct pollfd * pfd) {

	// Not end of file
	if (!feof(stream)) {

		// Read in a set of data from the file:
		fread(&values_unpack, sizeof(uint16_t), ADIS_DATA_LEN, stream);

		// Check for a carrage return for end of file and data sanity
		if (fread(&cr, sizeof(char), 1, stream) == 1) {

			// tmp
			char line[128];

			// Unpack data
			ADIS16405_burst_data data;
			data.adis_supply_out = values_unpack[0];
			data.adis_xgyro_out = values_unpack[1];
			data.adis_ygyro_out = values_unpack[2];
			data.adis_zgyro_out = values_unpack[3];
			data.adis_xaccl_out = values_unpack[4];
			data.adis_yaccl_out = values_unpack[5];
			data.adis_zaccl_out = values_unpack[6];
			data.adis_xmagn_out = values_unpack[7];
			data.adis_ymagn_out = values_unpack[8];
			data.adis_zmagn_out = values_unpack[9];
			data.adis_temp_out = values_unpack[10];
			data.adis_aux_adc = values_unpack[11];

			// dump data into socket
			memcpy(line, &data, sizeof(ADIS16405_burst_data));
			sendto_socket(sd, line, sizeof(ADIS16405_burst_data), FC_IP, FC_LISTEN_PORT);
		}
	}

	timerfd_settime(fd, 0, &t, NULL);	//set up next timer
}


void init_profiling() {
	// Set up a poll to get fake data
	pollfd_callback cb = NULL;
	t.it_interval.tv_sec = 0;
	t.it_interval.tv_nsec = 0;
	t.it_value.tv_sec = 0;
	t.it_value.tv_nsec = 1220703; // 819.2 samples per second
	//t.it_value.tv_nsec = 50000000; // Super slow, for debug
	fd = timerfd_create(CLOCK_MONOTONIC, 0);
	timerfd_settime(fd, 0, &t, NULL);

	cb = adis_sim_cb;
    sd = get_send_from_socket(ADIS_RX_PORT);

	// Add poll
	fcf_add_fd (fd, POLLIN, cb);

	// Init readin file
	stream = fopen(FCF_FAKE_ADIS_DATA_FILE, "rb");
}

void finalize_profiling() {

}
