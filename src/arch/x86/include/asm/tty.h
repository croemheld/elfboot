#ifndef __BOOT_TTY_H__
#define __BOOT_TTY_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include <asm/bios.h>

#include <elfboot/io.h>

void putc(int ch);

void puts(const char *str);

#endif /* __BOOT_TTY_H__ */