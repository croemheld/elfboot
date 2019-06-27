#include <elfboot/core.h>
#include <elfboot/mm.h>
#include <elfboot/device.h>
#include <elfboot/sections.h>
#include <elfboot/string.h>
#include <elfboot/printf.h>

#include <asm/boot.h>
#include <asm/bda.h>
#include <asm/video.h>
#include <asm/edd.h>

#include <uapi/elfboot/common.h>

#include <uapi/asm/bootparam.h>

static struct boot_params boot_params;

static void bootmem_reserve_regions(void)
{
	/* Reserve memory for the Interrupt Vector Table */
	memblock_reserve(IVT_ADDRESS, IVT_MAX_SIZE);

	/* Reserve memory for the BIOS Data Area */
	memblock_reserve(BDA_ADDRESS, BDA_MAX_SIZE);

	/* Reserve memory for the Extended BIOS Data Area */
	memblock_reserve(EBDA_ADDRESS, EBDA_MAX_SIZE);

	/* Reserve memory for boot stack */
	memblock_reserve(BOOT_STACK_ADDR_END, BOOT_STACK_SIZE);

	/* Reserve memory for the entire bootloader */
	memblock_reserve(SECTION_START(boot), SECTION_SIZE(boot));
}

/*
 * Architecture-specific main function
 */

int main(uint8_t disk_drive)
{
	struct device *bootdev;

	boot_params.disk_drive = disk_drive;

	/* Detect and set video modes */
	detect_videos(&boot_params);

	/* Get memory map */
	detect_memory(&boot_params);

	/* 
	 * Right after setting up our memblock allocator, we need 
	 * to reserve all regions which should not be allocated 
	 * in order to prevent overriding the bootloader and other 
	 * important structures in memory.
	 */
	bootmem_reserve_regions();

	/* Dump memory map */
	memblock_dump();

	/* SLOB memory allocator */
	bmalloc_init();

	/* Driver initialization */
	devices_init();

	/* Initialize boot device */
	bootdev = edd_device_create(disk_drive);
	if (!bootdev)
		return -EFAULT;

	device_mount(bootdev, "/dev/boot");

	bprintln("Initialized boot device");

	return 0;
}