#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/mm.h>
#include <elfboot/fs.h>
#include <elfboot/device.h>
#include <elfboot/printf.h>

#include <asm/boot.h>

static int init_fs_device(void)
{
	struct device *device = bmalloc(sizeof(*device));

	bprintln("Initialize rootfs...");

	if (!device)
		return -EFAULT;

	/*
	 * We create our root fs_node with a ramdisk and a ramfs
	 * filesystem in order to setup the filesystem hierarchy
	 * for our bootloader.
	 */
	
	/* Common properties */
	device->type = DEVICE_BLOCK;
	device_set(device, DEVICE_FLAG_VIRTUAL);
	device_set(device, DEVICE_FLAG_LBA);

	/* Device informations */
	device->info.interface = DEVICE_INTERFACE_RAMDISK;
	device->info.block_size = GENERIC_DEVICE_SECTOR_SIZE;

	/* Device root directory */
	device->device_data = bmalloc(GENERIC_DEVICE_SECTOR_SIZE);
	if (!device->device_data)
		goto fs_device_free_device;

	/* Initialization */
	if (fs_init(device))
		return -EFAULT;

	bprintln("Successfully created rootfs");

	return 0;

fs_device_free_device:
	bfree(device);

	return -ENOMEM;
}

int elfboot_main(void)
{
	/* Buddy allocation */
	page_alloc_init();

	/* SLUB memory allocator */
	bmalloc_init();

	/* Driver initialization */
	devices_init();

	/* Filesystem initialization */
	if (init_fs_device())
		return -EFAULT;

	/* Setup architecture */
	if (arch_init_late())
		return -EFAULT;

	return 0;
}