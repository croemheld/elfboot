#include <elfboot/core.h>
#include <elfboot/list.h>
#include <elfboot/device.h>
#include <elfboot/console.h>

int tty_probe(struct device *device __unused)
{
	return 0;
}

int tty_open(struct device *device __unused, const char *name __unused)
{
	return 0;
}

int tty_read(struct device *device __unused, uint64_t block __unused,
	     uint64_t size __unused, char *buffer __unused)
{
	return -ENOTSUP;
}

int tty_write(struct device *device, uint64_t block __unused, uint64_t size, 
	      const char *buffer)
{
	struct console *cons = device->data;

	return console_write(cons, buffer, size);
}

int tty_close(struct device *device __unused)
{
	return 0;
}

static struct device_driver tty_driver = {
	.name = "tty",
	.probe = tty_probe,
	.open = tty_open,
	.read = tty_read,
	.write = tty_write,
	.close = tty_close,
	.list = LIST_HEAD_INIT(tty_driver.list),
};

void init_tty_driver(void)
{
	device_driver_register(&tty_driver);
}

void fini_tty_driver(void)
{
	device_driver_unregister(&tty_driver);
}