
#ifndef ROLLSERVO_H_
#define ROLLSERVO_H_

#include "rollControlLibrary.h"

void rollservo_init(void);
void rollservo_final(void);

void sendRollServoData(char*);

void rs_getPositionData_rc(RollServo_adjustment*);



#endif /* ROLLSERVO_H_ */
