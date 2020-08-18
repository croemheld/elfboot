#ifndef __ELFBOOT_CONSOLE_H__
#define __ELFBOOT_CONSOLE_H__

#include <elfboot/core.h>
#include <elfboot/screen.h>

#define CONSOLE_CHAR_NEWLINE	'\n'
#define CONSOLE_CHAR_CRETURN	'\r'
#define CONSOLE_CHAR_SPACE		' '

#define CONSOLE_SCROLL_UP		0
#define CONSOLE_SCROLL_DOWN		1

struct console {
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