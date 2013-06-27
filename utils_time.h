/*
 * utils_time.h
 *
 */

#ifndef UTILS_TIME_H_
#define UTILS_TIME_H_
#include <time.h>

void utils_time_init(void);

void offset_start_time(struct timespec * ts);
void get_offset_time(struct timespec * ts);
void to_psas_time(struct timespec* ts, unsigned char* out);
void get_psas_time(unsigned char* out);
#endif /* UTILS_TIME_H_ */
