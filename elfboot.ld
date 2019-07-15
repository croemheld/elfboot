OUTPUT_FORMAT(binary)
ENTRY(_start)

SECTIONS
{
	. = 0x7C00;
	__boot_start = .;

	.boot ALIGN(0x10) : {
		*(.boot*)
	}

	. = 0x7DFE;
	.signature : {
		SHORT(0xAA55);
	}

	.real ALIGN(0x10) : {
		__real_start = .;
		*(.real*)
		__real_end = .;
	}

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

	/DISCARD/ : {
		*(.comment*)
	}

	__boot_end = .;
}