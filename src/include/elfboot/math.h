#ifndef __ELFBOOT_MATH_H__
#define __ELFBOOT_MATH_H__

#include <elfboot/core.h>
#include <elfboot/bitops.h>

static inline int log2(uint32_t val)
{
	return fls(val) - 1;
}

/*
 * https://stackoverflow.com/questions/600293
 */

static inline bool is_pow2(uint32_t val)
{
	return (val != 0 && ((val & (val - 1)) == 0));
}

static inline uint32_t round_up_pow2(uint32_t val)
{
	return (1UL << fls(val - 1));
}

static inline uint32_t round_down_pow2(uint32_t val)
{
	return (1UL << (fls(val) - 1));
}

#endif /* __ELFBOOT_MATH_H__ */