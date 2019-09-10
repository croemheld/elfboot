#include <elfboot/core.h>
#include <elfboot/mm.h>
#include <elfboot/device.h>
#include <elfboot/fs.h>
#include <elfboot/sections.h>
#include <elfboot/string.h>
#include <elfboot/printf.h>

#include <asm/boot.h>
#include <asm/bda.h>
#include <asm/pic.h>
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
	memblock_reserve(STACK_ADDR_END, STACK_SIZE);

	/* Reserve memory for the entire bootloader */
	memblock_reserve(SECTION_START(boot), SECTION_SIZE(boot));
}

/*
 * Architecture-specific main function
 */

int arch_init_late(void)
{
	struct device *bootdev;

	/* Initialize boot device */
	bootdev = edd_device_create(boot_params.disk_drive);
	if (!bootdev)
		return -EFAULT;

	if (device_create(bootdev, "cdrom"))
		return -EFAULT;

	if (fs_mount(bootdev, "/dev"))
		return -EFAULT;

	bprintln("Initialized boot device");

	return 0;
}

int arch_main(uint8_t disk_drive)
{
	boot_params.disk_drive = disk_drive;

	/* PIC remap */
	pic_init();

	/* Detect and set video modes */
	detect_videos(&boot_params);

	/* Hello World! */
	bprintln("Starting elfboot x86 bootloader...");

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

	/* Setup core */
	elfboot_main();

	return 0;
}