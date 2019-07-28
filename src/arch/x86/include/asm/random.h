#ifndef __X86_RANDOM_H__
#define __X86_RANDOM_H__

#include <elfboot/core.h>

#define RDRAND_NUMBER_TRYS	10

static inline bool arch_rdrand(void)
{
	/*
	 * Check if rdrand is supported
	 */
	return false;
}

static inline bool arch_has_rdrand(void)
{
	/*
	 * Check CPUID result for RDRAND instruction support
	 */
	return false;
}

static inline bool arch_get_random(uint32_t *random)
{
	return arch_has_rdrand() ? arch_rdrand(random) : false;
}

#endif /* __X86_RANDOM_H__ */