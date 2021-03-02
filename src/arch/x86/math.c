#include <elfboot/core.h>
#include <elfboot/math.h>

uint64_t arch_div(uint64_t val, uint32_t div, uint32_t *rem)
{
	uint32_t upp, low;
	uint64_t result = 0;

	/*
	 * Both gcc and clang use __udivdi3 and __umoddi3 to deal with divisions
	 * and modulo operations, which requires the compilers to link the files
	 * against their low level libc. This happens when the division uses two
	 * different types of numbers (uint64_t, uint32_t).
	 *
	 * To make the code work without the requirement ob a low level libc, we
	 * simply use the div assembly instruction.
	 */

	upp = val >> 32;
	low = val & (_BITULL(32) - 1);

	if (upp >= div) {
		result = (uint64_t)(upp / div) << 32;
		upp = (upp % div);
	}

	asm volatile("divl %2" : "=a"(low), "=d"(*rem) : "rm"(div), "0"(low), "1"(upp));

	return result | low;
}