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
	
	printf("%s sends: \n", src);
	for (int i = 0; i < len; i++) {
		char c = buffer[i];
		printf ("%X(%c) ", c, c < 32 ? '.': c);
	}
	printf("\n");
}

void getTemp_logger_screen(const char *src, char *buffer, int len) {
	// some data has been passed into this function for consumption.
	unsigned char msb = buffer[3];
	unsigned char lsb = buffer[2];
	unsigned short ret = msb<<CHAR_BIT|lsb;

	int offset = 18;

	double celc = ret/128;
	int fahr = (int)((celc * (9/5)) + 32 + offset);

	printf("\nTemp: %d F\n", fahr);
	printf("\n");
}

void getMouseMessage_logger_screen(const char *src, unsigned char *buffer, int len){
	char but = buffer[0];
	
	switch((int)but){
		case 1:
			printf("\n[%s] Mouse button(s): [X][0][0]\n", src);
			break;
		case 2:
			printf("\n[%s] Mouse button(s): [0][0][X]\n", src);
			break;
		case 3:
			printf("\n[%s] Mouse button(s): [X][0][X]\n", src);
			break;
		case 4:
			printf("\n[%s] Mouse button(s): [0][X][0]\n", src);
			break;
		case 5:
			printf("\n[%s] Mouse button(s): [X][X][0]\n", src);
			break;
		case 6:
			printf("\n[%s] Mouse button(s): [0][X][X]\n", src);
			break;
		case 7:
			printf("\n[%s] Mouse button(s): [X][X][X]\n", src);
			break;
		default: 
			printf("\n[%s] ---------\n", src);
			break;
		
	}
}

void finalize_logger_screen(){
}


// Other private functions to do stuff.
