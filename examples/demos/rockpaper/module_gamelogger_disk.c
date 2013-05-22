/**
 *  @file testLoggerDisk.c
 *  @brief Logs all info to a file on disk
 *  @details Has functions to initialize the disk logger, get messages to log to disk, and to "finalize" the disk logging by closing the file being written to.
 *  @author Clark Wachsmuth
 *  @date February 8th, 2013
 */


/***
*  testLoggerDisk.c
*  Test Module to Log data to Disk
*
*/

#include <stdio.h>

FILE * game_fp = NULL;

/**
 *  @brief Initializes the disk logging function
 *  @details Initializes function by copying filename into private data and opening the file. Error produced if file can't be opened.  
 *  @param filename Character pointer to name of file to be written to.
 */
void init_gamelogger_disk(void){
	game_fp = fopen("gamelogfile.log", "w+");

	if(!game_fp){
		printf("Could not open file for writing.\n");
	}
	setbuf(game_fp, NULL);
}

/**
 *  @brief Gets message to write to file.
 *  @param src Name of source
 *  @param buffer Message to be written
 *  @param len Length of data in buffer
 */
// Writes data to file.
void getGameMessage_gamelogger_disk(char *src, char *buffer, int len) {
	fprintf(game_fp, "[%s]\n%s\n", (const char *)src, (const char *)buffer);
} 

void getMouseMessage_gamelogger_disk(const char *src, unsigned char *buffer, int len) {
	//fprintf(game_fp, "%s: %s\n", src, buffer);
	if(len){
		if(buffer[0] > 0){
			fprintf(game_fp, "[%s]\nButton click %d\n", (const char *)src, (int)buffer[0]);
		}
	}
} 

/**
 *  @brief Closes the file stream.
 */
// Closes file stream.
void finalize_gamelogger_disk(){
	fclose(game_fp);
}
