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
#include "psas_packet.h"


void logger_init() {
	
}
void logger_final() {

}

void  log_getPositionData_adis(ADIS_packet *data) {
	// some data has been passed into this function for consumption.
	
	printf("Code: ");
        for (int i = 0; i < 4; i++) {
          printf("%c", data->ID[i]);
        }
        printf("\nTimestamp: ");
        for (int i = 0; i < 16; i++) {
          printf("%c", data->timestamp[i]);
        }
	printf("\n\n");
}
