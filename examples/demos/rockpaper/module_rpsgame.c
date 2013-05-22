
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "module_rpsgame.h"

#define ROCK	 1
#define PAPER	 4
#define SCISSORS 2
#define NUMTOWIN 5	/* The score needed to win the game */

#define true  1
#define false 0

/* Data Variables */
static int device_click[2], score[2], click_val[2];
static char * player_names[2];
static char game_buffer[2048];
static int length;

/* Functions */

static void resetTurn(void);
static void resetAll(void);
static char *returnValueName(int);
static int checkValues();
static void end_game(int, int);


static void setValidClick(int val, int dev){
	click_val[dev] = val;
	device_click[dev] = true;
	length = sprintf(game_buffer, "%s has decided.\n", player_names[dev]);
	sendMessage_rpsgame("rpsgame", game_buffer, length);
}


void getDeviceClick_rpsgame(const char *src, unsigned char *button, int dev){
	if(strcmp(src, "mouse_clark")==0)
		dev = 0;
	else
		dev = 1;

	if(!device_click[dev]){
		switch((int)button[0]){
			case ROCK: 
				setValidClick(ROCK, dev);
				break;
			case PAPER:
				setValidClick(PAPER, dev);
				break;
			case SCISSORS:
				setValidClick(SCISSORS, dev);
				break;
		}
		//printf("Mouse %d: %d\n", dev, (int)button[0]);
		checkValues();
	}
}

static char * returnValueName(int val){
	switch(val){
		case ROCK: 
			return "Rock";
		case PAPER:
			return "Paper";
		case SCISSORS:
			return "Scissors";
		default:
			return NULL;
	}
}

static int checkValues(){
	if(device_click[0] && device_click[1]){
		if(click_val[0]==click_val[1]){
			length = sprintf(game_buffer, "\n* Tie! Both players chose %s\n\n", returnValueName(click_val[0]));
			sendMessage_rpsgame("rpsgame", game_buffer, length);
			resetTurn();
		}
		else{
			int winner = 42;
			int loser = 42;
			if(click_val[0]==1 && click_val[1]==2){
				winner=0;
			}
			else if(click_val[0]==2 && click_val[1]==4){
				winner=0;
			}
			else if(click_val[0]==4 && click_val[1]==1){
				winner=0;
			}
			else{
				winner=1;
			}

			score[winner]++;

			if(winner){
				loser=0;
			}
			else{
				loser=1;
			}

			length = sprintf(game_buffer, "\n* %s wins matchup! %s beats %s\n\n\n", player_names[winner], returnValueName(click_val[winner]), returnValueName(click_val[loser]));
			sendMessage_rpsgame("rpsgame", game_buffer, length);
			sleep(1);

			if(score[winner] >= NUMTOWIN){
				end_game(winner, loser);
			}
			else{
				resetTurn();
			}

			
		}
		
		return 0;
	}
	else{
		return -1;
	}
}


static void end_game(int winner, int loser){
	length = sprintf(game_buffer, "\n\n\n\n********************************\n  %s defeats %s, %d to %d! \n********************************\n\n\n\n\n\n", player_names[winner], player_names[loser], score[winner], score[loser]);
	sendMessage_rpsgame("rpsgame", game_buffer, length);
	resetAll();
}


static void resetTurn(){
	device_click[0] = false;
	device_click[1] = false;
	click_val[0] = 0;
	click_val[1] = 0;
	length = sprintf(game_buffer, "[Score: %s: %d | %s: %d] Ready... Go!\n", player_names[0], score[0], player_names[1], score[1]);
	sendMessage_rpsgame("rpsgame", game_buffer, length);
}


static void resetAll(){
	score[0] = 0;
	score[1] = 0;
	resetTurn();
}


void init_rpsgame(){
	player_names[0] = "Player 1";
	player_names[1] = "Player 2";
	system ( "clear" );
	length = sprintf(game_buffer, "\n======================================================\n   WELCOME TO ROCK, PAPER, SCISSORS!!1!one!\n======================================================\n\n");
	sendMessage_rpsgame("rpsgame", game_buffer, length);
	resetAll();
}

void finalize_rpsgame(){
}