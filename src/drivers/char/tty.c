#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/mm.h>
#include <elfboot/module.h>
#include <elfboot/cdev.h>
#include <elfboot/string.h>
#include <elfboot/printf.h>

#include <drivers/tty.h>

static int tty_init(void)
{
	bprintln(DRIVER_TTY ": Initialize module...");

	return 0;
}

static void tty_exit(void)
{
	/*
	 * Not supported.
	 */

	bprintln(DRIVER_TTY ": Exit module...");
}

module_init(tty_init);
module_exit(tty_exit);