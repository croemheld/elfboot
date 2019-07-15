#ifndef __ELFBOOT_PRINTF_H__
#define __ELFBOOT_PRINTF_H__

#include <elfboot/core.h>
#include <elfboot/timestamp.h>

int vsnprintf(char *buffer, size_t size, const char *format, va_list *argp);

int snprintf(char *buffer, size_t size, const char *format, ...);

int sprintf(char* buffer, const char *format, ...);

int bprintf(const char *format, ...);

#define bprintln(fmt, ...)			\
({						\
	print_timestamp();			\
	bprintf(fmt "\n", ##__VA_ARGS__);	\
})

#endif /* __ELFBOOT_PRINTF_H__ */