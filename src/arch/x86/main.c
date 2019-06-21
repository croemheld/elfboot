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

static int bootdev_check_type(const char *interface_type, const char *type)
{
	return strncmp(interface_type, type, strlen(type));
}

static int bootdev_setup(struct device *device, struct edd_device_info *edi)
{
	const char *interface_type;
	struct edd_disk_drive_params *ddp;

	bprintf("Initialize bootdev... ");

	ddp = segment_offset_ptr(edi->params.dpte_ptr);

	device->params.io_base = ddp->io_base;
	device->params.control = ddp->control;

	/* Device flags */
	if (ddp->flags & EDD_DISK_DRIVE_PARAM_SLAVE)
		device_set_flag(device, DEVICE_FLAGS_SLAVE);

	if (ddp->flags & EDD_DISK_DRIVE_PARAM_LBA)
		device_set_flag(device, DEVICE_FLAGS_LBA);

	/* Device type */
	interface_type = edi->params.interface_type;

	if (!bootdev_check_type(interface_type, EDD_DEVICE_INTERFACE_ATA))
		device->params.type = DEVICE_ATA;

	if (!bootdev_check_type(interface_type, EDD_DEVICE_INTERFACE_ATAPI))
		device->params.type = DEVICE_ATAPI;

	if (device_install_firmware(device))
		return -EFAULT;

	bprintln("ok");

	return 0;
}

static int bootdev_initialize(void)
{
	struct edd_device_info edi;
	struct device *bootdev;

	if (edd_read_device_info(boot_params.disk_drive, &edi))
		return -EFAULT;

	bootdev = bmalloc(sizeof(*bootdev));
	if (!bootdev)
		return -ENOMEM;

	if (bootdev_setup(bootdev, &edi))
		return -EFAULT;

	return 0;
}

/*
 * Architecture-specific main function
 */

int main(uint8_t disk_drive)
{
	int r;

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
	r = bootdev_initialize();
	if (r)
		return r;

	return 0;
}