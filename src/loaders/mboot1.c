#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/mm.h>
#include <elfboot/file.h>
#include <elfboot/loader.h>
#include <elfboot/module.h>
#include <elfboot/string.h>
#include <elfboot/printf.h>

#include <loader/mboot1.h>

static void *multiboot_data = NULL;

static int mboot1_prepare_mem(struct multiboot_info *mbi)
{
	mbi->mem_lower = memory_lower_size();
	mbi->mem_upper = memory_upper_size();

	mbi->flags |= MBOOT1_INFO_MEMORY;

	return 0;
}

static int mboot1_prepare_dev(struct multiboot_info *mbi)
{
	mbi->boot_device = bootdev_partitions();

	mbi->flags |= MBOOT1_INFO_BOOTDEV;

	return 0;
}

static int mboot1_prepare_cmd(struct multiboot_info *mbi, struct boot_entry *boot_entry)
{
	uint32_t cmdsize = strlen(boot_entry->cmdline) + 1;

	/* Current address already set */
	mbi->cmdline = tuint(multiboot_data);

	/*
	 * We round up the size of the occupied multiboot information structure, so
	 * that each subsequent information is 4-byte aligned.
	 */
	memcpy(multiboot_data, boot_entry->cmdline, cmdsize);
	multiboot_data = vptradd(multiboot_data, round_up(cmdsize, 4));

	mbi->flags |= MBOOT1_INFO_CMDLINE;

	return 0;
}

static int mboot1_prepare_mod(struct boot_entry *boot_entry __unused,
	struct file *kernel __unused, struct mboot1_info *info __unused)
{
	return 0;
}

static int mboot1_prepare_sym(struct boot_entry *boot_entry __unused,
	struct file *kernel __unused, struct mboot1_info *info __unused)
{
	return 0;
}

static int mboot1_prepare_map(struct multiboot_info *info)
{
	return 0;
}

static int mboot1_prepare_drv(struct boot_entry *boot_entry __unused,
	struct file *kernel __unused, struct mboot1_info *info __unused)
{
	return 0;
}

static int mboot1_prepare_tab(struct boot_entry *boot_entry __unused,
	struct file *kernel __unused, struct mboot1_info *info __unused)
{
	return 0;
}

static int mboot1_prepare_bln(struct boot_entry *boot_entry __unused,
	struct file *kernel __unused, struct mboot1_info *info __unused)
{
	return 0;
}

static int mboot1_prepare_apm(struct boot_entry *boot_entry __unused,
	struct file *kernel __unused, struct mboot1_info *info __unused)
{
	return 0;
}

static int mboot1_prepare_vbe(struct multiboot_info *info)
{
	return 0;
}

static int mboot1_prepare_fbi(struct boot_entry *boot_entry __unused,
	struct file *kernel __unused, struct mboot1_info *info __unused)
{
	return 0;
}

static int mboot1_compare_signature(struct file *kernel,
	struct mboot1_info *info)
{
	uint32_t offset, *buffer;

	buffer = bmalloc(MBOOT1_SEARCH_LIMIT);
	if (!buffer)
		return -ENOMEM;

	if (!file_read(kernel, MBOOT1_SEARCH_LIMIT, buffer))
		goto mboot1_signature_fail;

	for (offset = 0; offset < MBOOT1_SEARCH_COUNT; offset++) {
		if (buffer[offset] != MBOOT1_HEADER_MAGIC)
			continue;

		info->header = bmalloc(MBOOT1_HEADER_SIZE);
		if (!info->header)
			return -ENOMEM;

		file_seek(kernel, FILE_SET, offset * sizeof(offset));
		if (!file_read(kernel, MBOOT1_HEADER_SIZE, info->header))
			goto mboot1_signature_fail;

		info->flags = info->header->flags;

		if (info->header->checksum != -(info->flags + MBOOT1_HEADER_MAGIC)) {
			bfree(info->header);
			goto mboot1_signature_fail;
		}

		bfree(buffer);

		return 0;
	}

mboot1_signature_fail:
	bfree(buffer);

	return -EFAULT;
}

static int mboot1_prepare_kernel(struct boot_entry *boot_entry,
	struct file *kernel, struct mboot1_info *info)
{
	Elf32_Ehdr *ehdr = bmalloc(sizeof(*ehdr));

	file_seek(kernel, FILE_SET, 0);
	if (!file_read(kernel, sizeof(*ehdr), ehdr))
		goto mboot1_kernel_fail;

	bprintln(DRIVER_MBOOT1 ": Found valid ELF object");

	return 0;

mboot1_kernel_fail:
	bfree(ehdr);

	return -EFAULT;
}

static int mboot1_prepare_info(struct boot_entry *boot_entry,
	struct file *kernel, struct mboot1_info *info)
{
	
	info->mbinfo = memblock_alloc_kernel(PAGE_SIZE, PAGE_SIZE);
	if (!info->mbinfo)
		return -ENOMEM;

	multiboot_data = vptradd(info->mbinfo, sizeof(struct multiboot_info));

	bprintln(DRIVER_MBOOT1 ": Allocated multiboot information structure at 0x%08lx", info->mbinfo);

	info->align = round_up(tuint(info->mbinfo) + sizeof(*info->mbinfo), 4);

	/*
	 * For our bootloader, it doesn't matter whether the first bit of the flags
	 * is set or not since we always align modules on a 4 KiB boundary, even if
	 * the bit is not set.
	 */

	if (mboot1_prepare_mem(info->mbinfo))
		return -EFAULT;

	if (mboot1_prepare_dev(info->mbinfo))
		return -EFAULT;

	if (mboot1_prepare_cmd(info->mbinfo, boot_entry))
		return -EFAULT;

	if (mboot1_prepare_mod(boot_entry, kernel, info))
		return -EFAULT;

	if (mboot1_prepare_sym(boot_entry, kernel, info))
		return -EFAULT;

	if (mboot1_prepare_map(info->mbinfo))
		return -EFAULT;

	if (mboot1_prepare_tab(boot_entry, kernel, info))
		return -EFAULT;

	if (mboot1_prepare_bln(boot_entry, kernel, info))
		return -EFAULT;

	if (mboot1_prepare_apm(boot_entry, kernel, info))
		return -EFAULT;

	if (mboot1_prepare_vbe(info->mbinfo))
		return -EFAULT;

	/*
	 * Starting from here, we decide which information we are going to tell the
	 * kernel.
	 */

	return 0;
}

static int mboot1_boot(struct boot_entry *boot_entry)
{
	struct file *kernel;
	struct mboot1_info info = { 0 };

	kernel = file_open(boot_entry->kernel_path, 0);
	if (!kernel)
		return -ENOENT;

	if (mboot1_compare_signature(kernel, &info))
		return -ENOENT;

	bprintln(DRIVER_MBOOT1 ": Found signature: %lx", MBOOT1_HEADER_MAGIC);

	if (mboot1_prepare_kernel(boot_entry, kernel, &info))
		return -EFAULT;

	if (mboot1_prepare_info(boot_entry, kernel, &info))
		return -EFAULT;

	// kernel_realmode_jump(rmcodeseg, rmcodeseg + 0x20);

	return 0;
}

static struct elf_loader mboot1_loader = {
	.prot = LOADER_PROTOCOL_MULTIBOOT1,
	.boot = mboot1_boot
};

static int mboot1_init(void)
{
	bprintln(DRIVER_MBOOT1 ": Initialize Multiboot 1 loader module...");

	loader_register(&mboot1_loader);

	return 0;
}

static void mboot1_exit(void)
{
	/*
	 * Not supported.
	 */

	bprintln(DRIVER_MBOOT1 ": Exit module...");
}

module_init(mboot1_init);
module_exit(mboot1_exit);
