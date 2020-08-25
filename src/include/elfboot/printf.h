#ifndef __ELFBOOT_PRINTF_H__
#define __ELFBOOT_PRINTF_H__

#include <elfboot/core.h>

/*
 * format_struct structure for passing arguments to the individual handlers
 * of strings, characters and numbers. This structure should always be used
 * because we don't like making our lifes complicated.
 */

typedef enum {
    PAD_NUL	= 0x00000001,
    PAD_NEG	= 0x00000002,
    SGN_POS	= 0x00000004
} format_flags_t;

struct format_struct {
	char *buf;
	uint32_t flags;

	/*
	 * Purposes of the following fields: pad, pre, len
	 *
	 * pad: Padding left or right f the printed string
	 * pre: Precision for strings, i.e.: string length
	 * len: Length of string to print, without padding
	 */
	int pad;
	int pre;
	int len;
};

int vsprintf(char *buffer, const char *format, va_list *argp);

int sprintf(char *buffer, const char *format, ...);

int bprintf(const char *format, ...);

int dprintf(const char *format, ...);

#define bprintln(fmt, ...)	bprintf(fmt "\n", ##__VA_ARGS__)

#endif /* __ELFBOOT_PRINTF_H__ */