#include <elfboot/core.h>
#include <elfboot/bitops.h>

#include <asm/math.h>

int log2(uint32_t val)
{
	return fls(val) - 1;
}

/*
 * https://stackoverflow.com/questions/600293
 */

bool is_pow2(uint32_t val)
{
	return (val != 0 && ((val & (val - 1)) == 0));
}

uint32_t round_up_pow2(uint32_t val)
{
	return (1UL << fls(val - 1));
}

uint32_t round_down_pow2(uint32_t val)
{
	return (1UL << (fls(val) - 1));
}

/*
 * Division with remainder
 */

uint64_t div(uint64_t val, uint32_t div, uint32_t *rem)
{
	return arch_div(val, div, rem);
}

/*
 * Power
 */

uint64_t pow(uint64_t base, uint64_t exp)
{
	uint64_t val;

	if (exp == 0)
		return 1;

	val = pow(base, exp / 2);
	if (!(exp % 2))
		return val * val;
	else
		return base * val * val;
}