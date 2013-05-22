/*
 * module_profile.h
 *
 */

#ifndef MODULE_PROFILE_H_
#define MODULE_PROFILE_H_

extern void init_profiling(void); // [miml:init]
extern void finalize_profiling(void); // [miml:final]
extern void getMessage_profile (unsigned char *buf, int len); // [miml:receiver]
extern void sendMessage_profile(unsigned char *buff, int length); // [miml:sender]
extern void sendMessage_profile3(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j, int k, int l, int m, int n, int o, int p, int q, int r, int s, int t, int u, int v, int w, int x, int y, int z); // [miml:sender]
extern void getMessage_profile3 (int a, int b, int c, int d, int e, int f, int g, int h, int i, int j, int k, int l, int m, int n, int o, int p, int q, int r, int s, int t, int u, int v, int w, int x, int y, int z); // [miml:sender]
#endif /* MODULE_PROFILE_H_ */
