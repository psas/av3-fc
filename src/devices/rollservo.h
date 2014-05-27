/*
 * Handles initializing and formatting messages for the roll servo board
 */


#ifndef ROLLSERVO_H_
#define ROLLSERVO_H_

#include "rollcontrol.h"

void MIML_INIT rollservo_init(void);
void MIML_FINAL rollservo_final(void);
void MIML_RECEIVER rs_receive_adj(RollServoMessage*);



#endif /* ROLLSERVO_H_ */
