#ifndef __ELFBOOT_CONSOLE_H__
#define __ELFBOOT_CONSOLE_H__

#include <elfboot/core.h>
#include <elfboot/screen.h>

#define CONSOLE_NEWLINE_CHAR	'\n'

struct console {
	int active;
	int xpos;
	int ypos;
	struct screen *screen;
};

void console_set_active(struct console *cons);

int console_write(struct console *cons, const char *str, size_t len);

int console_write_active(const char *str, size_t len);

void console_init(struct screen *screen);

#endif /* __ELFBOOT_CONSOLE_H__ */