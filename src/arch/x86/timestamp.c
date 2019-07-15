#include <elfboot/core.h>
#include <elfboot/printf.h>

static uint64_t tsc = 0;
static uint64_t tsc_start = 0;

static inline uint64_t current_timestamp(void)
{
	asm volatile("rdtsc" : "=A"(tsc));

	return tsc;
}

void print_timestamp(void)
{
	tsc = current_timestamp();

	if (!tsc_start)
		tsc_start = tsc;

	bprintf("[%10llu.0] ", tsc - tsc_start);
}