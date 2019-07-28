#ifndef __X86_BITOPS_H__
#define __X86_BITOPS_H__

#include <elfboot/core.h>

static inline int ffs(uint32_t val)
{
	int r;

	asm(
		"bsfl    %1, %0		\n\t"
		"jnz     1f		\n\t"
		"movl    $-1, %0	\n"

		"1:" : "=r" (r) : "rm" (val)
	);

	return r + 1;
}

static inline int fls(uint32_t val)
{
	int r;

	asm(
		"bsrl    %1, %0		\n\t"
		"jnz     1f		\n\t"
		"movl    $-1, %0	\n"

		"1:" : "=r" (r) : "rm" (val)
	);

	return r + 1;
}

#endif /* __X86_BITOPS_H__ */