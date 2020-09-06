#include <elfboot/core.h>
#include <elfboot/mm.h>
#include <elfboot/initcall.h>
#include <elfboot/fs.h>
#include <elfboot/pci.h>
#include <elfboot/bdev.h>
#include <elfboot/file.h>
#include <elfboot/input.h>
#include <elfboot/interrupts.h>
#include <elfboot/loader.h>
#include <elfboot/module.h>
#include <elfboot/symbol.h>
#include <elfboot/string.h>
#include <elfboot/printf.h>

#include <asm/boot.h>

static int symbols_init(void)
{
	struct file *file;
	void *syms;

	file = file_open("/root/elfboot.map", 0);
	if (!file)
		return -ENOENT;

	syms = bmalloc(file->length);
	if (!syms)
		return -ENOMEM;

	if (!file_read(file, file->length, syms))
		goto symbols_free_syms;

	return symbol_parse_map(syms);

symbols_free_syms:
	bfree(syms);

	return -EFAULT;
}

static int modules_load(void)
{
	/*
	 * TODO CRO: Load modules depending on the architecture
	 *
	 * E.g. "pit" only available for x86
	 */

#if CONFIG_DRIVER_TTY == CONFIG_M
	if (module_open("tty"))
		return -EFAULT;
#endif /* CONFIG_DRIVER_TTY */

	if (module_open("pit"))
		return -EFAULT;

	if (module_open("kbd"))
		return -EFAULT;

	return 0;
}

/*
 * elfboot modinit function for initializing built-in modules. This function
 * uses the extern variables declared in the linker file to initialize 
 */
int elfboot_init(modinit_t *start, modinit_t *end)
{
	modinit_t modinit, *function = start;

	for (; function < end; function++) {
		modinit = *function;

		if (modinit())
			return -EFAULT;
	}

	return 0;
}

int elfboot_main(void)
{
	/* Buddy allocation */
	if (page_alloc_init())
		return -EFAULT;

	/* SLUB memory allocator */
	if (bmalloc_init())
		return -EFAULT;

	/* VFS initialization */
	if (vfs_init())
		return -EFAULT;

	/* PCI devices */
	if (pci_init())
		return -EFAULT;

	/* Built-in modules */
	if (modules_init())
		return -EFAULT;

	/* Call arch-specific late init function */
	if (arch_init_late())
		return -EFAULT;

	/* Parse symbol map */
	if (symbols_init())
		return -EFAULT;

	/* Load external modules */
	if (modules_load())
		return -EFAULT;

	/* Initialize loader */
 	if (loader_init())
 		return -EFAULT;

	return 0;
}