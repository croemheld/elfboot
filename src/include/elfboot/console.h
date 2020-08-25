#ifndef __ELFBOOT_CONSOLE_H__
#define __ELFBOOT_CONSOLE_H__

#include <elfboot/core.h>
#include <elfboot/input.h>

#define CONSOLE_CHAR_NEWLINE	'\n'
#define CONSOLE_CHAR_CRETURN	'\r'
#define CONSOLE_CHAR_SPACE		' '

#define CONSOLE_CTRL_ENTER		CONSOLE_CHAR_NEWLINE
#define CONSOLE_CTRL_ARRUP		KEY_ARROW_UP
#define CONSOLE_CTRL_ARRDO		KEY_ARROW_DOWN

#define CONSOLE_SCROLL_UP		0
#define CONSOLE_SCROLL_DOWN		1

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

#define COLOR_VGA_BLACK			0
#define COLOR_VGA_BLUE			1
#define COLOR_VGA_GREEN			2
#define COLOR_VGA_CYAN			3
#define COLOR_VGA_RED			4
#define COLOR_VGA_PURPLE		5
#define COLOR_VGA_BROWN			6
#define COLOR_VGA_GRAY			7
#define COLOR_VGA_DARKGRAY		8
#define COLOR_VGA_LIGHTBLUE		9
#define COLOR_VGA_LIGHTGREEN	10
#define COLOR_VGA_LIGHTCYAN		11
#define COLOR_VGA_LIGHTRED		12
#define COLOR_VGA_LIGHTPURPLE	13
#define COLOR_VGA_YELLOW		14
#define COLOR_VGA_WHITE			15

#define COLOR_VGA(x)			(struct color){ .vga.color = (x) }

struct color {
	union {
		struct rgb_color rgb;
		struct hex_color hex;
		struct vga_color vga;
	};
};

struct console {
	uint32_t active;
	uint32_t fbaddr;

	/*
	 * Height and width of the console in units of characters. We only support
	 * printing text to the screen which is why an unit is represented by char
	 */
	uint32_t width;
	uint32_t height;

	/*
	 * Number of lines for one character in frame buffer mode. Only applicable
	 * for consoles which print to the screen.
	 */
	uint32_t xunit;
	uint32_t yunit;

	/*
	 * Current position in the console depicted by coordinates. BPU (bytes per
	 * unit) is the number of bytes used to display a single character.
	 */
	uint32_t xpos;
	uint32_t ypos;
	uint32_t bpu;

	/*
	 * Colors for both foreground and background. Foreground colors only refer
	 * to the color of fonts while background colors for everything behind and
	 * between the fonts.
	 */
	struct color fgcolor;
	struct color bgcolor;
};

struct console_attr {
	uint32_t active;
	uint32_t width;
	uint32_t height;
	uint32_t xpos;
	uint32_t ypos;
	struct color fgcolor;
	struct color bgcolor;
};

static inline void *console_unit(struct console *cons, uint32_t xpos, uint32_t ypos)
{
	int lpos = xpos + cons->width * ypos;

	return tvptr(cons->fbaddr + cons->bpu * lpos);
}

static inline void *console_line(struct console *cons, uint32_t ypos)
{
	return console_unit(cons, 0, ypos);
}

static inline size_t console_bpl(struct console *cons)
{
	return cons->width * cons->bpu;
}

int console_init(const char *path);

#endif /* __ELFBOOT_CONSOLE_H__ */