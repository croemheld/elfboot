OUTPUT_FORMAT(elf32-i386)
ENTRY(_arch_start)

SECTIONS
{
	/*
	 * Start of the bootloader text section. This has to be placed at this
	 * offset since we continue directly after the boot sector.
	 */

	. = 0x7E00;
	__bootstrap_start = .;

	.text ALIGN(0x10) : {
		__text_start = .;
		*(.text*)
		__text_end = .;
	}

	/*
	 * This section is dedicated to all built-in modules. If a module will
	 * be included in the elfboot binary, the module_init function pointer
	 * of that module is placed in this section.
	 */

	.modinit ALIGN(0x10) : {
		/* Built-in file systems */
		__modinit_vfs_start = .;
		KEEP(*(.modinit_vfs*))
		__modinit_vfs_end = .;

		/* Built-in devices */		
		__modinit_dev_start = .;
		KEEP(*(.modinit_dev*))
		__modinit_dev_end = .;

		/* Built-in modules */		
		__modinit_start = .;
		KEEP(*(.modinit*))
		__modinit_end = .;
	}

	.modexit ALIGN(0x10) : {
		__modexit_start = .;
		KEEP(*(.modexit*))
		__modexit_end = .;
	}

	/*
	 * Data sections. We have to place the .rodata section at the very end
	 * of the file since the ordering of the sections may introduce a huge
	 * gap with null bytes which are contained in the final binary.
	 */

	.bss ALIGN(0x10) : {
		__bss_start = .;
		*(.bss*)
		__bss_end = .;
	}

	.data ALIGN(0x10) : {
		__data_start = .;
		*(.data*)
		__data_end = .;
	}

	.rodata ALIGN(0x10) : {
		__rodata_start = .;
		*(.rodata*)
		__rodata_end = .;
	}

	__bootstrap_end = .;
}
