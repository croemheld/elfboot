#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/mm.h>
#include <elfboot/file.h>
#include <elfboot/loader.h>
#include <elfboot/module.h>
#include <elfboot/string.h>
#include <elfboot/printf.h>

#include <asm/segment.h>

#include <loader/lxboot.h>

static int lxboot_prepare_fields(struct boot_entry *boot_entry,
	struct lxboot_info *info)
{
	struct lxboot_rm_header *rmcodehdr = info->rmcodehdr;

	if (info->bprotvers >= 0x0200) {
		uint16_t heap_end;

		rmcodehdr->type_of_loader = 0xff;

		if (info->bprotvers >= 0x0202 && rmcodehdr->loadflags & 0x01)
			heap_end = 0xe000;
		else
			heap_end = 0x9800;

		if (info->bprotvers >= 0x0201) {
			rmcodehdr->heap_end_ptr = heap_end - 0x200;
			rmcodehdr->loadflags |= 0x80;
		}

		if (info->bprotvers >= 0x0202) {
			rmcodehdr->cmd_line_ptr = tuint(info->rmcodebuf) + heap_end;
			strcpy(tvptr(rmcodehdr->cmd_line_ptr), boot_entry->cmdline);
		} else {
			return -ENOTSUP;
		}
	} else {
		return -ENOTSUP;
	}

	return 0;
}

static int lxboot_prepare_kernel(struct boot_entry *boot_entry,
	struct file *kernel, struct lxboot_info *info)
{
	struct lxboot_rm_header *rmcodehdr;

	rmcodehdr = bmalloc(sizeof(*rmcodehdr));
	if (!rmcodehdr)
		return -EFAULT;

	file_lseek(kernel, FILE_SET, 0x1f1);
	if (!file_read(kernel, sizeof(*rmcodehdr), rmcodehdr))
		return -EFAULT;

	if (rmcodehdr->header != LXBOOT_BZIMAGE_SIGNATURE)
		return -EFAULT;

	info->signature = rmcodehdr->header;
	info->bprotvers = rmcodehdr->version;
	info->rmcodesec = rmcodehdr->setup_sects;
	if (!info->rmcodesec)
		info->rmcodesec = 4;

	bfree(rmcodehdr);

	info->rmcodelen = (info->rmcodesec << 9) + 512;
	info->rmcodebuf = bmalloc(info->rmcodelen);
	if (!info->rmcodebuf)
		return -ENOMEM;

	file_lseek(kernel, FILE_SET, 0);
	if (!file_read(kernel, info->rmcodelen, info->rmcodebuf))
		return -EFAULT;

	info->rmcodehdr = vptradd(info->rmcodebuf, 0x1f1);
	if (lxboot_prepare_fields(boot_entry, info))
		return -EFAULT;

	info->rmcodehdr->vid_mode = 0xffff;
	info->pmcodelen = kernel->length - info->rmcodelen;

	file_lseek(kernel, FILE_SET, info->rmcodelen);
	if (!file_read(kernel, info->pmcodelen, tvptr(LXBOOT_KERNEL_ADDR)))
		return -EFAULT;

	bprintln(DRIVER_LXBOOT ": Boot protocol version: %04x", info->bprotvers);

	return 0;
}

static int lxboot_prepare_initrd(struct file *initrd, struct lxboot_info *info)
{
	struct lxboot_rm_header *rmcodehdr = info->rmcodehdr;

	if (!file_read(initrd, initrd->length, tvptr(LXBOOT_INITRD_ADDR)))
		return -EFAULT;

	rmcodehdr->ramdisk_image = LXBOOT_INITRD_ADDR;
	rmcodehdr->ramdisk_size = initrd->length;

	return 0;
}

static int lxboot_prepare_farjmp(struct lxboot_info *info)
{
	uint16_t rmcodeseg = RM_SEG(tuint(info->rmcodebuf));

	kernel_realmode_jump(rmcodeseg, rmcodeseg + 0x20);

	return -EFAULT;
}

static int lxboot_boot(struct boot_entry *boot_entry)
{
	struct file *kernel, *initrd;
	struct lxboot_info info = { 0 };

	kernel = file_open(boot_entry->kernel_path, 0);
	if (!kernel)
		return -ENOENT;

	initrd = file_open(boot_entry->initrd_path, 0);
	if (!initrd)
		return -ENOENT;

	if (lxboot_prepare_kernel(boot_entry, kernel, &info))
		return -EFAULT;

	if (lxboot_prepare_initrd(initrd, &info))
		return -EFAULT;

	if (lxboot_prepare_farjmp(&info))
		return -EFAULT;

	return 0;
}

static struct elf_loader lxboot_loader = {
	.prot = LOADER_PROTOCOL_LINUX,
	.boot = lxboot_boot
};

static int lxboot_init(void)
{
	bprintln(DRIVER_LXBOOT ": Initialize Linux loader module...");

	loader_register(&lxboot_loader);

	return 0;
}

static void lxboot_exit(void)
{
	/*
	 * Not supported.
	 */

	bprintln(DRIVER_LXBOOT ": Exit module...");
}

module_init(lxboot_init);
module_exit(lxboot_exit);