#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/mm.h>
#include <elfboot/io.h>
#include <elfboot/module.h>
#include <elfboot/console.h>
#include <elfboot/file.h>
#include <elfboot/cdev.h>
#include <elfboot/string.h>
#include <elfboot/printf.h>

#include <drivers/tty.h>

/*
 * Current coordinates in the TTY
 */
static struct console tty_console = {
	.fbaddr = TTY_BASE_ADDRESS,
	.width  = TTY_MAX_WIDTH,
	.height = TTY_MAX_HEIGHT,
	.xunit  = 1,
	.yunit  = 1,
	.bpu = 2
};

static int tty_read(struct cdev *cdev, uint64_t offset, uint64_t length,
	void *buffer)
{
	return -ENOTSUP;
}

static uint16_t tty_char(char c)
{
	return (uint16_t)c | (0x07 << 8);
}

static void tty_clear(struct console *cons)
{
	uint16_t *dst, val;

	dst = console_line(cons, 0);
	val = tty_char(CONSOLE_CHAR_SPACE);

	memset16(dst, val, console_bpl(cons) * TTY_MAX_HEIGHT);
}

static void tty_cursor_update(struct console *cons)
{
	uint16_t lpos = cons->xpos + cons->ypos * cons->width;

	outb(0x3D4, 0x0F);
	outb(0x3D5, (lpos >> 0) & 0xFF);
	outb(0x3D4, 0x0E);
	outb(0x3D5, (lpos >> 8) & 0xFF);
}

static void tty_scroll(struct console *cons, int direction, int units)
{
	void *dst, *src;

	if (!units || direction != CONSOLE_SCROLL_UP)
		return;

	dst = console_line(cons, 0);
	src = console_line(cons, units);

	memmove(dst, src, console_bpl(cons) * (cons->height - units));

	dst = console_line(cons, cons->height - units);

	memset16(dst, tty_char(CONSOLE_CHAR_SPACE), console_bpl(cons) * units);
}

static int __tty_write(struct console *cons, uint64_t length, const char *buffer)
{
	uint64_t index;
	uint16_t *unit;

	for (index = 0; index < length && buffer[index]; index++) {
		/* Handle TTY scrolling case */
		if (cons->ypos >= cons->height) {
			tty_scroll(cons, CONSOLE_SCROLL_UP, 1);
			cons->ypos -= cons->yunit;
		}

		tty_cursor_update(cons);

		/* Handle different characters */
		if (buffer[index] == CONSOLE_CHAR_NEWLINE) {
			cons->xpos  = 0;
			cons->ypos += cons->yunit;
		} else if (buffer[index] != CONSOLE_CHAR_CRETURN) {
			unit = console_unit(cons, cons->xpos++, cons->ypos);
			memset16(unit, tty_char(buffer[index]), 1);

			if (cons->xpos >= TTY_MAX_WIDTH) {
				cons->xpos = 0;
				cons->ypos++;
			}
		}
	}

	return 0;
}

static int tty_write(struct cdev *cdev, uint64_t offset, uint64_t length,
	const void *buffer)
{
	return __tty_write(cdev->private, length, buffer);
}

static int tty_ioctl(struct cdev *cdev, int request, void *args)
{
	return 0;
}

static struct cdev_ops tty_cdev_ops = {
	.read = tty_read,
	.write = tty_write,
	.ioctl = tty_ioctl
};

static int tty_init(void)
{
	struct cdev *cdev;

	cdev = bmalloc(sizeof(*cdev));
	if (!cdev)
		return -ENOMEM;

	cdev->name = bstrdup("tty");
	if (!cdev->name)
		goto tty_free_cdev;

	cdev->private = &tty_console;

	if (cdev_init(cdev, &tty_cdev_ops))
		goto tty_free_name;

	tty_clear(cdev->private);

	return console_init("/dev/tty");

tty_free_name:
	bfree_const(cdev->name);

tty_free_cdev:
	bfree(cdev);

	return -EFAULT;
}

static void tty_exit(void)
{
	/*
	 * Not supported.
	 */

	bprintln(DRIVER_TTY ": Exit module...");
}

dev_module_init(tty_init);
dev_module_exit(tty_exit);