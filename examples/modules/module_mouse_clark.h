/*
 * module_mouse_clark.h
 *
 */

#ifndef MODULE_MOUSE_CLARK_H_
#define MODULE_MOUSE_CLARK_H_

void init_mouse_clark(void); // [miml:init]
void finalize_mouse_clark(void); // [miml:final]
extern void sendMessage_mouse_clark(const char *src, unsigned char *buffer, int length); // [miml:sender]

#endif /* MODULE_MOUSE_CLARK_H_ */
