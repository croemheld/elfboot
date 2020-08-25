#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/mm.h>
#include <elfboot/io.h>
#include <elfboot/module.h>
#include <elfboot/console.h>
#include <elfboot/file.h>
#include <elfboot/cdev.h>
#include <elfboot/math.h>
#include <elfboot/string.h>
#include <elfboot/printf.h>
#include <elfboot/debug.h>
#include <elfboot/libtmg.h>

#include <drivers/tty.h>

#include <uapi/elfboot/ioctls.h>

#define VGA_COLOR(fgcolor, bgcolor)	\
	(fgcolor).vga.color | (bgcolor).vga.color << 4

/*
 * Current coordinates in the TTY
 */
static struct console tty_console = {
	.active = 1,
	.fbaddr = TTY_BASE_ADDRESS,
	.width  = TTY_MAX_WIDTH,
	.height = TTY_MAX_HEIGHT,
	.xunit  = 1,
	.yunit  = 1,
	.bpu = 2,
	.fgcolor = COLOR_VGA(COLOR_VGA_GRAY),
	.bgcolor = COLOR_VGA(COLOR_VGA_BLACK)
};

/*
 * TTY utilities
 */

static uint16_t tty_char(struct console *cons, char c)
{
	uint16_t color = VGA_COLOR(cons->fgcolor, cons->bgcolor);

	return (uint16_t)c | color << 8;
}

static void tty_cursor_update(struct console *cons)
{
	uint16_t lpos = cons->xpos + cons->ypos * cons->width;

	outb(0x3D4, 0x0F);
	outb(0x3D5, (lpos >> 0) & 0xFF);
	outb(0x3D4, 0x0E);
	outb(0x3D5, (lpos >> 8) & 0xFF);
}

static void tty_cursor_enable(struct console *cons)
{
	outb(0x3D4, 0x0A);
	outb(0x3D5, (inb(0x3D5) & 0xC0) | 0x0E);
	outb(0x3D4, 0x0B);
	outb(0x3D5, (inb(0x3D5) & 0xE0) | 0x0F);

	tty_cursor_update(cons);

	cons->active = 1;
}

static void tty_cursor_disable(struct console *cons)
{
	tty_cursor_update(cons);

	outb(0x3D4, 0x0A);
	outb(0x3D5, 0x20);

	cons->active = 0;
}

/*
 * TTY IOCTLs
 */

static void tty_paint(struct console *cons, struct tmg_header *tmg)
{
	uint16_t xpos, ypos;
	uint16_t maxw, maxh;
	uint16_t word, *plot, *unit;

	maxw = min(TTY_MAX_WIDTH, (cons->xpos + tmg->width));
	maxh = min(TTY_MAX_HEIGHT, cons->ypos + tmg->height);

	for (ypos = 0; cons->ypos + ypos < maxh; ypos += 2) {
		for (xpos = 0; cons->xpos + xpos < maxw; xpos++) {
			/*
			 * Every time we want to print something, we have to fetch the plot
			 * (the encoded character) from the file for which we are forced to
			 * move the pointer in the file.
			 */
			word = sizeof(*tmg) + (ypos * tmg->width) + (xpos * cons->bpu);
			plot = vptradd(tmg, word);
			unit = console_unit(cons, cons->xpos + xpos, cons->ypos + ypos / 2);
			memset16(unit, *plot, 2);
		}
	}
}

static void tty_reset(struct console *cons, struct console_attr *attr)
{
	if (!attr)
		return;

	cons->xpos = attr->xpos;
	cons->ypos = attr->ypos;

	cons->fgcolor = attr->fgcolor;
	cons->bgcolor = attr->bgcolor;

	if (attr->active)
		tty_cursor_enable(cons);
}

static void tty_clean(struct console *cons, struct console_attr *attr)
{
	uint16_t *dst, val;

	tty_reset(cons, attr);

	dst = console_line(cons, 0);
	val = tty_char(cons, CONSOLE_CHAR_SPACE);

	memset16(dst, val, console_bpl(cons) * TTY_MAX_HEIGHT);

	if (!attr)
		tty_cursor_disable(cons);
	else
		tty_reset(cons, attr);
}

static void tty_gattr(struct console *cons, struct console_attr *attr)
{
	attr->active = cons->active;
	attr->width  = cons->width;
	attr->height = cons->height;
	attr->xpos = cons->xpos;
	attr->ypos = cons->ypos;
	attr->fgcolor = cons->fgcolor;
	attr->bgcolor = cons->bgcolor;
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

	memset16(dst, tty_char(cons, CONSOLE_CHAR_SPACE), console_bpl(cons) * units);
}

static int tty_read(struct cdev *cdev, uint64_t offset, uint64_t length,
	void *buffer)
{
	return -ENOTSUP;
}

static int __tty_write(struct console *cons, uint64_t offset, uint64_t length,
	const char *buffer)
{
	uint64_t index;
	uint16_t *unit;

	for (index = 0; index < length && buffer[index]; index++) {
		/* Handle TTY scrolling case */
		if (cons->ypos >= cons->height) {
			tty_scroll(cons, CONSOLE_SCROLL_UP, 1);
			cons->ypos -= cons->yunit;
		}

		if (cons->active)
			tty_cursor_update(cons);

		/* Handle different characters */
		if (buffer[index] == CONSOLE_CHAR_CRETURN)
			cons->xpos  = 0;
		else if (buffer[index] == CONSOLE_CHAR_NEWLINE) {
			cons->xpos  = 0;
			cons->ypos += cons->yunit;
		} else if (buffer[index] != CONSOLE_CHAR_CRETURN) {
			unit = console_unit(cons, cons->xpos++, cons->ypos);
			memset16(unit, tty_char(cons, buffer[index]), 2);

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
	struct console *cons = cdev->private;

	if (offset)
		cons->ypos = div_u64_rem(offset, TTY_MAX_WIDTH, &cons->xpos);

	return __tty_write(cons, offset, length, buffer);
}

static int tty_ioctl(struct cdev *cdev, int request, void *args)
{
	switch (request) {
		case IOCTL_RESET:
			tty_reset(cdev->private, args);
			break;
		case IOCTL_CLEAN:
			tty_clean(cdev->private, args);
			break;
		case IOCTL_PAINT:
			tty_paint(cdev->private, args);
			break;
		case IOCTL_GATTR:
			tty_gattr(cdev->private, args);
			break;
		default:
			return -ENOTSUP;
	}

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

	tty_clean(cdev->private, NULL);

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