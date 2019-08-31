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

/*
 * Division with remainder
 */

static inline uint64_t div_u64_rem(uint64_t dividend, uint32_t divisor,
				   uint32_t *remainder)
{
	union {
		uint64_t v64;
		uint32_t v32[2];
	} d = { dividend };
	uint32_t upper;

	upper = d.v32[1];
	d.v32[1] = 0;

	if (upper >= divisor) {
		d.v32[1] = upper / divisor;
		upper %= divisor;
	}

	asm ("divl %2" : "=a" (d.v32[0]), "=d" (*remainder) :
		"rm" (divisor), "0" (d.v32[0]), "1" (upper));

	return d.v64;
}

#endif /* __ELFBOOT_MATH_H__ */