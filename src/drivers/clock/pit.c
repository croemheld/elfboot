#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/mm.h>
#include <elfboot/io.h>
#include <elfboot/input.h>
#include <elfboot/interrupts.h>
#include <elfboot/module.h>
#include <elfboot/string.h>
#include <elfboot/printf.h>

#include <drivers/pit.h>

static int pit_init(void)
{
	uint32_t divisor = DRIVER_PIT_FREQUENCY / 1000;

	bprintln(DRIVER_PIT ": Initialize module...");

	outb(0x40, (divisor >> 0) & 0xFF);
	outb(0x40, (divisor >> 8) & 0xFF);

	enable_clock();

	return 0;
}

static void pit_exit(void)
{
	/*
	 * Not supported.
	 */

	bprintln(DRIVER_PIT ": Exit module...");

	disable_clock();
}

module_init(pit_init);
module_exit(pit_exit);