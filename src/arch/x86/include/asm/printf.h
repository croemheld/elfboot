#ifndef __BOOT_PRINTF_H__
#define __BOOT_PRINTF_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

int vsnprintf(char *buffer, size_t size, const char *format, va_list *argp);

int snprintf(char *buffer, size_t size, const char *format, ...);

int sprintf(char* buffer, const char *format, ...);

int bprintf(const char *format, ...);

#endif /* __BOOT_PRINTF_H__ */