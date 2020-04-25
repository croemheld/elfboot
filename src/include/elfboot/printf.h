#ifndef __ELFBOOT_PRINTF_H__
#define __ELFBOOT_PRINTF_H__

#include <elfboot/core.h>

int vsprintf(char *buffer, const char *format, va_list *argp);

int sprintf(char *buffer, const char *format, ...);

int bprintf(const char *format, ...);

#define bprintln(fmt, ...)	bprintf(fmt "\n", ##__VA_ARGS__)

#endif /* __ELFBOOT_PRINTF_H__ */