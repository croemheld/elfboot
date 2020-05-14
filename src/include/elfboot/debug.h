#ifndef __ELFBOOT_DEBUG_H__
#define __ELFBOOT_DEBUG_H__

#include <elfboot/core.h>
#include <elfboot/linkage.h>

#include <elfboot/debug/qemu.h>
#include <elfboot/debug/bochs.h>

/*
 * TODO CRO: Introduce debug functions
 */

static __always_inline void debug_printf(const char *str, int len)
{
#ifdef CONFIG_DEBUG_QEMU
	qemu_bprintf(str, len);
#endif
#ifdef CONFIG_DEBUG_BOCHS
	bochs_bprintf(str, len);
#endif
}

#endif /* __ELFBOOT_DEBUG_H__ */