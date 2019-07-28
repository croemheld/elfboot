#ifndef __ELFBOOT_LINKAGE_H__
#define __ELFBOOT_LINKAGE_H__

#include <elfboot/core.h>

#include <asm/linkage.h>

/*
 * Sections
 */

#define __section(x)			__attribute__((section(x)))

/*
 * Macro to place real mode stubs at the beginning
 * of the generated binary file in order to ensure
 * that it is accessible in real mode.
 *
 * Only neccessary if bootloader oversteps the max
 * size allowed (within 64KB).
 */
#define __real_function			__section(".real.functions")

/*
 * Structures and functions
 */

#define __unused			__attribute__((unused))
#define __packed			__attribute__((packed))

/*
 * Alignment
 */

#define __aligned(x)			__attribute__((aligned(x)))

/*
 * Inline
 */
#define __always_inline			inline __attribute__((always_inline))

#endif /* __ELFBOOT_LINKAGE_H__ */