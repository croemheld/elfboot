#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/string.h>
#include <elfboot/screen.h>
#include <elfboot/io.h>
#include <elfboot/list.h>

static inline uint16_t vga_char_color(struct screen *screen)
{
	struct vga_color fg, bg;

	fg = screen->fgcolor.vga;
	bg = screen->bgcolor.vga;

	return fg.color | bg.color << 4;
}

static inline uint16_t vga_char_value(struct screen *screen, unsigned char c)
{
	return (uint16_t)c | vga_char_color(screen) << 8;
}

static void vga_fill_line(struct screen *screen, int y, unsigned char c)
{
	uint16_t value, *line;

	line  = screen_line(screen, y);
	value = vga_char_value(screen, c);

	memset16(line, value, screen->width * screen->bpu * screen->xunit);
}


static int vga_clear(struct screen *screen)
{
	int i;

	for (i = 0; i < screen->height; i++)
		vga_fill_line(screen, i, SCREEN_SPACE_CHAR);

	return screen->height;
}

/*
 * TODO CRO: Can me make this function public for all drivers?
 */
static int vga_scroll(struct screen *screen, int direction, int units)
{
	void *dst, *src;
	uint16_t val;
	int lines;

	if (!units)
		return 0;

	if (direction != SCROLL_UP)
		return -ENOTSUP;

	dst = screen_line(screen, 0);
	src = screen_line(screen, units);
	val = vga_char_value(screen, SCREEN_SPACE_CHAR);

	lines = screen->height - units;
	memmove(dst, src, screen_bpl(screen) * lines);

	dst = screen_line(screen, lines);

	memset16(dst, val, screen_bpl(screen) * units);

	return units;
}

static int vga_putc(struct screen *screen, char c, int x, int y)
{
	uint16_t *unit = screen_unit(screen, x, y);

	*unit = vga_char_value(screen, c);

	return 1;
}

/*
 * TODO CRO: Any other approach?
 */
static int vga_update_cursor(struct screen *screen, int x, int y)
{
	uint16_t lpos = x + y * screen->width;

	outb(0x3D4, 0x0F);
	outb(0x3D5, (lpos >> 0) & 0xff);
	outb(0x3D4, 0x0E);
	outb(0x3D5, (lpos >> 8) & 0xff);

	return 0;
}

static struct screen_ops vga_driver = {
	.name = "VGA",
	.clear = vga_clear,
	.scroll = vga_scroll,
	.putc = vga_putc,
	.update_cursor = vga_update_cursor,
	.list = LIST_HEAD_INIT(vga_driver.list)
};

void vga_driver_init(void)
{
	screen_driver_register(&vga_driver);
}

void vga_driver_fini(void)
{
	screen_driver_unregister(&vga_driver);
}