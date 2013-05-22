/*
 * module_fakemouse.h
 *
 */

#ifndef FAKEMOUSE_H_
#define FAKEMOUSE_H_

void init_fakemouse(void);
void finalize_fakemouse(void);
void getMessage_fakemouse(unsigned char *buf, int len);

#endif /* FAKEMOUSE_H_ */
