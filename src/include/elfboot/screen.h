#ifndef __DRIVER_SCREEN_H__
#define __DRIVER_SCREEN_H__

#include <elfboot/core.h>
#include <elfboot/list.h>

#include <uapi/elfboot/common.h>

#define SCREEN_SPACE_CHAR		' '

enum scroll_direction {
	SCROLL_UP,
	SCROLL_DOWN,
};

/*
 * The struct rgb_color and struct hex_color are congruent
 * so that it is possible to use either value at the same 
 * time.
 */

struct rgb_color {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	uint8_t alpha;
	uint8_t b;
	uint8_t g;
	uint8_t r;
#else /* __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__*/
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t alpha;
#endif
};

struct hex_color {
	uint32_t color;
};

/*
 * The struct vga_color only holds 16 possible values ranging
 * from 0 (BLACK) to 15 (WHITE).
 */

struct vga_color {
	uint8_t color;
};

#define SCREEN_COLOR_VGA_BLACK		0
#define SCREEN_COLOR_VGA_BLUE		1
#define SCREEN_COLOR_VGA_GREEN		2
#define SCREEN_COLOR_VGA_CYAN		3
#define SCREEN_COLOR_VGA_RED		4
#define SCREEN_COLOR_VGA_PURPLE		5
#define SCREEN_COLOR_VGA_BROWN		6
#define SCREEN_COLOR_VGA_GRAY		7
#define SCREEN_COLOR_VGA_DARKGRAY	8
#define SCREEN_COLOR_VGA_LIGHTBLUE	9
#define SCREEN_COLOR_VGA_LIGHTGREEN	10
#define SCREEN_COLOR_VGA_LIGHTCYAN	11
#define SCREEN_COLOR_VGA_LIGHTRED	12
#define SCREEN_COLOR_VGA_LIGHTPURPLE	13
#define SCREEN_COLOR_VGA_YELLOW		14
#define SCREEN_COLOR_VGA_WHITE		15

struct screen_color {
	union {
		struct rgb_color rgb;
		struct hex_color hex;
		struct vga_color vga;
	};
};

#define SCREEN_COLOR_VGA(color)		{ (color) }

/*
 * Screen structure:
 *
 * The struct screen stores information about a screen 
 * in its respective units. For a VGA screen, the width 
 * and height refer to the columns and rows, as opposed
 * to a VESA video mode, where the same fields refer to
 * the number of pixels.
 *
 * As of now, we only support linear framebuffers.
 */

struct screen;

struct screen_ops {
	const char *name;
	int (*clear)(struct screen *);
	int (*scroll)(struct screen *, int, int);
	int (*putc)(struct screen *, char, int, int);
	int (*update_cursor)(struct screen *, int, int);
	struct list_head list;
};

struct screen {
	const char *name;
	int width;
	int height;
	int xunit;
	int yunit;
	int bpu;
	struct screen_color fgcolor;
	struct screen_color bgcolor;
	uint32_t fbaddr;
	struct screen_ops *ops;
};

/*
 * Returns the address of a given unit in the screen.
 */
static inline void *screen_unit(struct screen *screen, int xpos, int ypos)
{
	int lpos = xpos + screen->width * ypos;

	return uinttvptr(screen->fbaddr + screen->bpu * lpos);
}

/*
 * Returns the address of the start of a line in the screen.
 */
static inline void *screen_line(struct screen *screen, int ypos)
{
	return screen_unit(screen, 0, ypos);
}

int screen_clear(struct screen *screen);

int screen_scroll(struct screen *screen, int direction, int units);

int screen_putc(struct screen *screen, char c, int x, int y);

int screen_update_cursor(struct screen *screen, int x, int y);

int screen_setup(struct screen *screen);

void screen_init(void);

void screen_driver_register(struct screen_ops *ops);

void screen_driver_unregister(struct screen_ops *ops);

#endif /* __DRIVER_SCREEN_H__ */