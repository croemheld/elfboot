#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/device.h>
#include <elfboot/string.h>
#include <elfboot/list.h>

static int random_probe(struct device *device __unused)
{
	return -ENOTSUP;
}

static int random_open(struct device *device __unused, const char *name __unused)
{
	return -ENOTSUP;
}

static int random_read(struct device *device __unused, uint64_t sector __unused, 
		       uint64_t size __unused, char *buffer __unused)
{
	return -ENOTSUP;
}

static int random_write(struct device *device __unused, uint64_t sector __unused,
			uint64_t size __unused, const char *buffer __unused)
{
	return -ENOTSUP;
}

static int random_close(struct device *device __unused)
{
	return -ENOTSUP;
}

struct device_driver random_device_driver = {
	.type = DEVICE_RANDOM,
	.probe = random_probe,
	.open = random_open,
	.read = random_read,
	.write = random_write,
	.close = random_close,
	.list = LIST_HEAD_INIT(random_device_driver.list),
};

void rand_firmware_init(void)
{
	device_driver_register(&random_device_driver);
}