/*
 * Handles initializing and formatting messages for the roll servo board
 */


#ifndef ROLLSERVO_H_
#define ROLLSERVO_H_

#include "rollcontrol.h"

void rollservo_init(void);
void rollservo_final(void);

void rs_receive_adj(ROLLMessage*);



#endif /* ROLLSERVO_H_ */
