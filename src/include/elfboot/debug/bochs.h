#ifndef __ELFBOOT_DEBUG_BOCHS_H__
#define __ELFBOOT_DEBUG_BOCHS_H__

#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/io.h>

static __always_inline void bochs_bp(void)
{
	outw(0x8A00, 0x8A00);
	outw(0x8A00, 0x8AE0);
}

static __always_inline void bochs_putc(char c)
{
	outb(0xE9, c);
}

static __always_inline void bochs_bprintf(const char *str, int len)
{
	int i;

	for (i = 0; i < len; i++)
		bochs_putc(str[i]);
}

#endif /* __ELFBOOT_DEBUG_BOCHS_H__ */