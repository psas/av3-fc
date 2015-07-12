/*
 * Policy for whether messages should be logged only to disk or also over Wi-Fi.
 */

#ifndef _FILTER_H_
#define _FILTER_H_

#include <stdint.h>

void filter_receive(const char ID[4], uint8_t timestamp[6], uint16_t data_length, void *data);
void filter_to_ground(const char ID[4], uint8_t timestamp[6], uint16_t data_length, void *data);
void filter_to_disk(const char ID[4], uint8_t timestamp[6], uint16_t data_length, void *data);

#endif
