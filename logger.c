/**
 *  @file testLoggerScreen.c
 *  @brief logs all info into a terminal session.
 *  @details Simple piece of code that passes data to be written to the active terminal session.
 *  @author Clark Wachsmuth
 *  @date February 8th, 2013
 */

#include <stdio.h>
#include <limits.h>
#include "adis.h"
#include "logger.h"


void logger_init() {
	
}
void logger_final() {

}

void  log_getPositionData_adis(ADIS_packet *data) {
	// some data has been passed into this function for consumption.
	
	printf("%s sends: \n", src);
	for (int i = 0; i < len; i++) {
		char c = buffer[i];
		printf ("%X(%c) ", c, c < 32 ? '.': c);
	}
	printf("\n");
}
