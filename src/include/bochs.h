#ifndef __UAPI_ELFBOOT_DEBUG_H__
#define __UAPI_ELFBOOT_DEBUG_H__

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

static __always_inline void bochs_bprintf(const char *buffer, int length)
{
	int i;

	for (i = 0; i < length; i++)
		bochs_putc(buffer[i]);
}

#endif /* __UAPI_ELFBOOT_DEBUG_H__ */