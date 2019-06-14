#include <elfboot/core.h>
#include <elfboot/mm.h>
#include <elfboot/console.h>
#include <elfboot/screen.h>

static struct console root_console = {
	.active = 1,
	.xpos = 0,
	.ypos = 0,
};

static struct console *active_console = &root_console;

static void store_console(struct console *cons __unused)
{
	/*
	 * TODO CRO: Console switching
	 *
	 * Allocate region to store content of console screen
	 */
}

void console_set_active(struct console *cons)
{
	store_console(active_console);

	active_console = cons;
}

static void console_handle_newline(struct console *cons)
{
	cons->xpos  = 0;
	cons->ypos += cons->screen->yunit;

	if (cons->ypos == cons->screen->height) {
		screen_scroll(cons->screen, SCROLL_UP, 1);
		cons->ypos -= cons->screen->yunit;
	}
}

int console_write(struct console *cons, const char *str, size_t len)
{
	size_t i;

	if (!cons->screen || !str)
		return 0;

	for (i = 0; i < len; i++) {
		if (str[i] == CONSOLE_NEWLINE_CHAR) {
			console_handle_newline(cons);

			continue;
		}

		screen_putc(cons->screen, str[i], cons->xpos, cons->ypos);

		/* VGA unit = 1, VBE unit depends on font size */
		cons->xpos += cons->screen->xunit;

		if (cons->xpos == cons->screen->width)
			console_handle_newline(cons);
	}

	if (cons->active)
		screen_update_cursor(cons->screen, cons->xpos, cons->ypos);

	return i;
}

int console_write_active(const char *str, size_t len)
{
	return console_write(active_console, str, len);
}

void console_init(struct screen *screen)
{
	/* Register drivers */
	screen_init();

	/* Setup root console */
	if (screen_setup(screen))
		return;

	root_console.screen = screen;
}