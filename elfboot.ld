OUTPUT_FORMAT(elf32-i386)
ENTRY(_arch_start)

SECTIONS
{
	/*
	 * The boot stack starts at 0x6000 and grows towards lower addresses.
	 * The stack is designed to be only one page in size, which should be
	 * sufficient for the bootstrap stage.
	 */

	. = 0x6000;
	__stack_start = .;

	/*
	 * Buffer for reading files from the boot device. Usually boot devices
	 * are designed to read data in chunks of 0x200 (512) or 0x800 (2048)
	 * bytes. The buffer is large enough to read 4 512 or 2 2048 chunks of
	 * data from the disk.
	 */

	__buffer_start = .;

	. = 0x7000;
	__buffer_end = .;

	/*
	 * Actual bootstrap stage code and data.
	 */

	. = 0x7E00;
	__bootstrap_start = .;

	.text ALIGN(0x10) : {
		__text_start = .;
		*(.text*)
		__text_end = .;
	}

	.rodata ALIGN(0x10) : {
		__rodata_start = .;
		*(.rodata*)
		__rodata_end = .;
	}

	/*
	 * This section is dedicated to all built-in modules. If a module will
	 * be included in the elfboot binary, the module_init function pointer
	 * of that module is placed in this section.
	 *
	 * Make sure to initialize built-in filesystems first as we need it to
	 * setup the root file system node.
	 */

	.initcalls ALIGN(0x10) : {
		__initcalls_vfs_start = .;

		/* Built-in file systems */
		*(.initcalls_vfs*)

		__initcalls_vfs_end = .;
		__initcalls_dev_start = .;

		/* Built-in devices */
		*(.initcalls_dev*)

		__initcalls_dev_end = .;
		__initcalls_start = .;

		/* Built-in modules */
		*(.initcalls*)

		__initcalls_end = .;
	}

	.exitcalls ALIGN(0x10) : {
		__exitcalls_start = .;
		*(.exitcalls*)
		__exitcalls_end = .;
	}

	.data ALIGN(0x10) : {
		__data_start = .;
		*(.data*)
		__data_end = .;
	}

	.bss ALIGN(0x10) : {
		__bss_start = .;
		*(.bss*)
		__bss_end = .;
	}

	__bootstrap_end = .;
}
