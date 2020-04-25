#ifndef __ELFBOOT_LINKAGE_H__
#define __ELFBOOT_LINKAGE_H__

#include <elfboot/core.h>
#include <elfboot/compiler.h>

#include <asm/linkage.h>

typedef int  (*initcall_t)(void);
typedef void (*exitcall_t)(void);

/*
 * Sections
 */
#define __section(x)		__attribute__((section(x)))

/*
 * Section macros
 */

/*
 * Macro to place real mode stubs at the beginning
 * of the generated binary file in order to ensure
 * that it is accessible in real mode.
 *
 * Only neccessary if bootloader oversteps the max
 * size allowed (within 64KB).
 */
#define __real_function		__section(".real.functions")

/*
 * Structures and functions
 */
#define __unused			__attribute__((unused))
#define __packed			__attribute__((packed))

/*
 * Alias
 */
#define __alias(x)			__attribute__((alias(x)))

/*
 * Alignment
 */
#define __aligned(x)		__attribute__((aligned(x)))

/*
 * Inline
 */
#define __always_inline		inline __attribute__((always_inline))

/*
 * The following macros are intended to be used for modules
 */
#define __modinit			__section(".initcalls")
#define __modexit			__section(".exitcalls")

#endif /* __ELFBOOT_LINKAGE_H__ */