#ifndef AV3_LOGGING_H
#define AV3_LOGGING_H

#include <stdint.h>
#include <netinet/in.h>		/* for htonl() */

#define FOURCC(a,b,c,d) htonl(((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

//copied and renamed glib macros
#define FCF_PASTE_ARGS(identifier1,identifier2) identifier1 ## identifier2
#define FCF_PASTE(identifier1,identifier2)      FCF_PASTE_ARGS (identifier1, identifier2)
#define FCF_STRINGIFY(macro_or_string)	FCF_STRINGIFY_ARG (macro_or_string)
#define	FCF_STRINGIFY_ARG(contents)	#contents

//GOptionGroup *options_logging(void);
void init_logging(void);
void flush_buffers(void);
void write_tagged_message(uint32_t fourcc, const void *buf, uint16_t len);
void printf_tagged_message(uint32_t fourcc, const char *fmt, ...);
void printbuffer(uint32_t fourcc, const unsigned char *buffer, int act_len);

#endif /* AV3_LOGGING_H */
