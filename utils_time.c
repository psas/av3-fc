/*
 * utils_time.c
 *
 */
#include <time.h>
#include <stdint.h>

struct timespec starttime; // TODO: timer module

void utils_time_init(){
	clock_gettime(CLOCK_REALTIME, &starttime);
}

void offset_start_time(struct timespec * ts){
    ts->tv_sec -= starttime.tv_sec;
    ts->tv_nsec -= starttime.tv_nsec;
}

void get_offset_time(struct timespec * ts){
	clock_gettime(CLOCK_REALTIME, ts);
	offset_start_time(ts);
}

void to_psas_time(struct timespec* ts, unsigned char* out){
	uint64_t now = ts->tv_nsec + (uint64_t) ts->tv_sec * 1000000000;

	out[0] = now >> 40;
	out[1] = now >> 32;
	out[2] = now >> 24;
	out[3] = now >> 16;
	out[4] = now >> 8;
	out[5] = now;
}

void get_psas_time(unsigned char* out){
	struct timespec ts;
	get_offset_time(&ts);
	to_psas_time(&ts, out);
}

