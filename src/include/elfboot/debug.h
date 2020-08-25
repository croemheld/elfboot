#ifndef __ELFBOOT_DEBUG_H__
#define __ELFBOOT_DEBUG_H__

#include <elfboot/core.h>
#include <elfboot/linkage.h>

#include <elfboot/debug/qemu.h>
#include <elfboot/debug/bochs.h>

static __always_inline void debug_breakp(void)
{
#ifdef CONFIG_DEBUG_BOCHS
	/*
	 * Bochs only supports x86 emulation, no need for implementing several ways
	 * for different architectures.
	 */
	asm volatile("xchg %bx, %bx");
#endif
#ifdef CONFIG_DEBUG_QEMU
	/*
	 * QEMU does not support magic breakpoints like Bochs, which is why we have
	 * to use a simple never ending loop to completely halt execution.
	 */
	while (true);
#endif
}

static __always_inline void debug_printf(const char *str, int len)
{
#ifdef CONFIG_DEBUG_BOCHS
	bochs_bprintf(str, len);
#endif
#ifdef CONFIG_DEBUG_QEMU
	qemu_bprintf(str, len);
#endif
}

#endif /* __ELFBOOT_DEBUG_H__ */