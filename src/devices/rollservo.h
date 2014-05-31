/*
 * Handles initializing and formatting messages for the roll servo board
 */


#ifndef ROLLSERVO_H_
#define ROLLSERVO_H_

#include <ev.h>
#include "rollcontrol.h"

void MIML_INIT rollservo_init(struct ev_loop * loop);
void MIML_FINAL rollservo_final(void);
void MIML_RECEIVER rs_receive_adj(RollServoMessage*);



#endif /* ROLLSERVO_H_ */
