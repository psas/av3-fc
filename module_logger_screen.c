/**
 *  @file testLoggerScreen.c
 *  @brief logs all info into a terminal session.
 *  @details Simple piece of code that passes data to be written to the active terminal session.
 *  @author Clark Wachsmuth
 *  @date February 8th, 2013
 */

#include <stdio.h>
#include <limits.h>

#include "module_logger_screen.h"


void init_logger_screen() {
	
}

// src... name of source
// buffer... message
// len... length of data in buffer

void getMessage_logger_screen(const char *src, unsigned char *buffer, int len) {
	// some data has been passed into this function for consumption.
	
	printf("%s sends: %4s\n", src, (const char *)buffer);
}

void finalize_logger_screen(){
}


// Other private functions to do stuff.
