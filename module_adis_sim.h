/*
 * module_adistest.h
 *
 */

#ifndef MODULE_PROFILE_H_
#define MODULE_PROFILE_H_

extern void init_profiling(void); // [miml:init]
extern void finalize_profiling(void); // [miml:final]
extern void sendMessage_profile(unsigned char *buff, int length); // [miml:sender]
#endif /* MODULE_PROFILE_H_ */
