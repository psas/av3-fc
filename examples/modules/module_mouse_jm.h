/*
 * module_mouse_jm.h
 *
 */

#ifndef MODULE_MOUSE_JM_H_
#define MODULE_MOUSE_JM_H_

void init_mouse_jm (void); // [miml:init]
void finalize_mouse_jm (void); // [miml:final]
extern void sendMessage_mouse_jm(const char *src, unsigned char *buffer, int length); // [miml:sender]

#endif /* MODULE_MOUSE_JM_H_ */
