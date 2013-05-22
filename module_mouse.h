/*
 * module_mouse.h
 *
 */

#ifndef MODULE_MOUSE_H_
#define MODULE_MOUSE_H_

void init_mouse(void); // [miml:init]
void finalize_mouse(void); // [miml:final]
extern void sendMessage_mouse(const char *src, unsigned char *buffer, int length); // [miml:sender]

#endif /* MODULE_MOUSE_H_ */
