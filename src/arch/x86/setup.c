#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/mm.h>
#include <elfboot/fs.h>
#include <elfboot/interrupts.h>
#include <elfboot/memblock.h>
#include <elfboot/sections.h>
#include <elfboot/string.h>
#include <elfboot/module.h>
#include <elfboot/bdev.h>
#include <elfboot/pci.h>
#include <elfboot/tree.h>
#include <elfboot/printf.h>

#include <crypto/crc32.h>

#include <asm/boot.h>
#include <asm/bda.h>
#include <asm/pic.h>
#include <asm/segment.h>

#include <uapi/asm/bootparam.h>

static struct boot_params boot_params = { 0 };

static int arch_init_boot_verify(struct bdev *bdev)
{
	char *uidsym, *buffer;
	uint32_t uidlba, uidlen;
	struct boot_info_table *bit;

	buffer = bmalloc(bdev->block_size);
	if (!buffer)
		return -ENOMEM;

	bit = boot_params.boot_table;
	bit->bootuid_lba = bit->elfboot_lba + bdev_blknum(bdev, bit->elfboot_len);
	uidlba = bit->bootuid_lba;
	uidlen = bdev_blknum(bdev, bit->bootuid_len);

	if (bdev_read(bdev, uidlba, uidlen, buffer))
		goto verify_free_buffer;

	uidsym = strtok(buffer, " ");
	if (strcmp(uidsym, ELFBOOT))
		goto verify_free_buffer;

	uidsym = strtok(NULL, " ");
	if (strtoul(uidsym, NULL, 16) != crc32(ELFBOOT, 7))
		goto verify_free_buffer;

	bdev->flags |= BDEV_FLAGS_BOOT;

	bfree(buffer);

	return 0;

verify_free_buffer:
	bfree(buffer);

	return -EFAULT;
}

static int arch_init_bootdevice(struct boot_params *boot_params)
{
	struct fs_node *node, *npos;

	node = vfs_open("/dev");
	if (!node)
		return -ENOENT;

	tree_for_each_child_entry(npos, node, tree) {

		/*
		 * We are only interested in block devices, which is why we simply skip
		 * those nodes that don't have that specific flag set.
		 */
		if (!(npos->flags & FS_BLOCKDEVICE))
			continue;

		if (!arch_init_boot_verify(npos->bdev))
			break;
	}

	if (!npos || !(npos->flags & FS_BLOCKDEVICE) || !npos->bdev)
		return -ENODEV;

	if (vfs_mount(npos->bdev, "/", "root"))
		return -EFAULT;

	return 0;
}

/*
 * arch_init_late: Second-stage late arch specific main function
 *
 * Initialize boot device and rootfs so that we can finally load
 * the kernel and jump to it afterwards.
 */

int arch_init_late(void)
{
	/*
	 * Initialize generic interrupt handler system before setting up the IDT
	 * so that we can register interrupt handlers as early as possible.
	 */
	if (init_interrupts())
		return -EFAULT;

	/*
	 * If we want to enable interrupts we have to set up the IDT here. We do
	 * this only if we intend to handle keyboard interrupts and other things
	 * to handle user input for e.g. boot menu.
	 */
	if (arch_init_interrupts())
		return -EFAULT;

	/*
	 * Initialize boot device and load the appropiate disk driver
	 * from it by using the information stored in boot info table
	 */
	if (arch_init_bootdevice(&boot_params))
		return -EFAULT;

	/* Detect and set video modes */
	if (detect_videos(&boot_params))
		return -EFAULT;

	return 0;
}

static void bootmem_reserve_regions(void)
{
	/* Reserve memory for the Interrupt Vector Table */
	memblock_reserve(IVT_ADDRESS, IVT_MAX_SIZE);

	/* Reserve memory for the BIOS Data Area */
	memblock_reserve(BDA_ADDRESS, BDA_MAX_SIZE);

	/* Reserve memory for the Extended BIOS Data Area */
	memblock_reserve(EBDA_ADDRESS, EBDA_MAX_SIZE);

	/* Reserve memory for boot stack */
	memblock_reserve(BOOT_STACK_START, BOOT_STACK_SIZE);

	/* Reserve memory for the entire bootloader */
	memblock_reserve(SECTION_START(bootstrap), SECTION_SIZE(bootstrap));
}

/* arch_setup: Second-stage prot mode arch specific main function
 *
 * The goal of this function is to retrieve as much information
 * as possible from the underlying machine. This is required to
 * make this bootloader multiboot compliant.
 */

int arch_setup(uint16_t disk_drive, struct boot_info_table *bit)
{
	boot_params.boot_table = bit;
	boot_params.disk_drive = disk_drive;

	/*
	 * The most important thing first: We need memory!
	 *
	 * Use the e820 memory map BIOS function to get a detailed map
	 * of available regions for allocations on this system.
	 */
	if (detect_memory(&boot_params))
		return -EFAULT;

	/* 
	 * Right after setting up our memblock allocator, we need 
	 * to reserve all regions which should not be allocated 
	 * in order to prevent overriding the bootloader and other 
	 * important structures in memory.
	 */
	bootmem_reserve_regions();

	/*
	 * Preparation done, call elfboot main function
	 */
	elfboot_main();

	/* That should not have happened... */
	return -EFAULT;
}