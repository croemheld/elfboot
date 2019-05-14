#include <asm/boot.h>
#include <asm/bda.h>
#include <asm/video.h>
#include <asm/edd.h>
#include <asm/printf.h>

#include <elfboot/disk.h>
#include <elfboot/device.h>
#include <elfboot/mm.h>

#include <uapi/asm/bootparam.h>

struct boot_params boot_params;

static void bootdev_startup_hook(void)
{
	struct device *bootdev;

	/* Initialize firmware drivers */
	disk_firmware_init();

	/* Device the bootloader was loaded from */
	bootdev = device_create(boot_params.disk_drive);

	bprintf("Boot device at %p\n", bootdev);
}

void bmain(uint8_t disk_drive)
{
	boot_params.disk_drive = disk_drive;

	/* Get memory map */
	bootmem_init(&boot_params);

	/* Initialize boot devices */
	bootdev_startup_hook();
}