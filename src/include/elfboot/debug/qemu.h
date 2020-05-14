#ifndef __ELFBOOT_DEBUG_QEMU_H__
#define __ELFBOOT_DEBUG_QEMU_H__

#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/io.h>

#define COM1_PORT	0x3f8

static __always_inline void qemu_putc(char c)
{
	while (!(inb(COM1_PORT + 5) & 0x20));

	outb(COM1_PORT, c);
}

static __always_inline void qemu_bprintf(const char *str, int len)
{
	int i;

	for (i = 0; i < len; i++)
		qemu_putc(str[i]);
}

#endif /* __ELFBOOT_DEBUG_QEMU_H__ */