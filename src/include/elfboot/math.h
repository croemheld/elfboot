#ifndef __ELFBOOT_MATH_H__
#define __ELFBOOT_MATH_H__

#include <elfboot/core.h>
#include <elfboot/bitops.h>

#include <asm/math.h>

int log2(uint32_t val);

/*
 * https://stackoverflow.com/questions/600293
 */

bool is_pow2(uint32_t val);

uint32_t round_up_pow2(uint32_t val);

uint32_t round_down_pow2(uint32_t val);

/*
 * Division with remainder
 */

uint64_t div(uint64_t val, uint32_t div, uint32_t *rem);

/*
 * Power
 */

uint64_t pow(uint64_t base, uint64_t exp);

#endif /* __ELFBOOT_MATH_H__ */