#ifndef AV3_LOGGING_H
#define AV3_LOGGING_H

#include <stdint.h>

#define FOURCC(a,b,c,d) htonl(((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

void init_logging(void);
void flush_buffers(void);
void write_tagged_message(uint32_t fourcc, const char *buf, uint16_t len);
void printf_tagged_message(uint32_t fourcc, const char *fmt, ...);

#endif /* AV3_LOGGING_H */
