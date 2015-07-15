/*
 * Utilities for converting to and from the PSAS time format
 *
 * The PSAS time format is 6 bytes long number in nanoseconds
 */

#ifndef UTILS_TIME_H_
#define UTILS_TIME_H_

#include <stdint.h>
#include <time.h>

void utils_time_init(void);

void to_psas_time(struct timespec* ts, unsigned char* out);
uint64_t from_psas_time(const unsigned char* in);
void get_psas_time(unsigned char* out);
#endif /* UTILS_TIME_H_ */
