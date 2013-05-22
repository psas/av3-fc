/*
 * gps.h
 *
 */

#ifndef GPS_H_
#define GPS_H_


void set_gps_devicepath(const char *dev);
void init_gps(void); // [miml:init]
void finalize_gps(void); // [miml:final]
extern void sendMessage_gps(unsigned char ret, unsigned char * buff, int length); // [miml:sender]

#endif /* GPS_H_ */
