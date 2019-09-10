#ifndef __ELFBOOT_CONSOLE_H__
#define __ELFBOOT_CONSOLE_H__

#include <elfboot/core.h>
#include <elfboot/screen.h>

#define CONSOLE_NEWLINE_CHAR	'\n'

#define CONSOLE_STATE_INACTIVE	0
#define CONSOLE_STATE_ACTIVE	1

struct console {
	int active;
	int xpos;
	int ypos;
	struct screen *screen;
};

void console_set_active(struct console *cons);

int console_write(struct console *cons, const char *str, size_t len);

int console_write_active(const char *str, size_t len);

#define console_printf(str, len)	console_write_active(str, len)

void console_init(struct screen *screen);

#endif /* __ELFBOOT_CONSOLE_H__ */