#include <elfboot/core.h>
#include <elfboot/string.h>
#include <elfboot/screen.h>
#include <elfboot/list.h>

#include <drivers/vga.h>

LIST_HEAD(screen_drivers);

int screen_scroll(struct screen *screen, int direction, int units)
{
	if (screen->ops->scroll)
		return screen->ops->scroll(screen, direction, units);

	return -ENOTSUP;
}

int screen_putc(struct screen *screen, char c, int x, int y)
{
	if (screen->ops->putc)
		return screen->ops->putc(screen, c, x, y);

	return -ENOTSUP;
}

int screen_update_cursor(struct screen *screen, int x, int y)
{
	if (screen->ops->update_cursor)
		return screen->ops->update_cursor(screen, x, y);

	return -ENOTSUP;
}

int screen_setup(struct screen *screen)
{
	struct screen_ops *ops;

	list_for_each_entry(ops, &screen_drivers, list) {
		if (strcmp(ops->name, screen->name))
			continue;

		screen->ops = ops;
	}

	return (screen->ops ? 0 : -EFAULT);
}

void screen_init(void)
{
	vga_driver_init();
}

void screen_driver_register(struct screen_ops *ops)
{
	list_add(&ops->list, &screen_drivers);
}

void screen_driver_unregister(struct screen_ops *ops)
{
	list_del(&ops->list);
}