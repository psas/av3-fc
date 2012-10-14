#ifndef AV3_LOGGING_H
#define AV3_LOGGING_H

#include <glib.h>
#include <stdint.h>
#include <netinet/in.h>		/* for htonl() */

#define FOURCC(a,b,c,d) htonl(((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

GOptionGroup *options_logging(void);
void init_logging(void);
void flush_buffers(void);
void write_tagged_message(uint32_t fourcc, const void *buf, uint16_t len);
void printf_tagged_message(uint32_t fourcc, const char *fmt, ...);

#endif /* AV3_LOGGING_H */
