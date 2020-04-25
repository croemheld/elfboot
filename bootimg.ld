OUTPUT_FORMAT(elf32-i386)
ENTRY(_start)

SECTIONS
{
	. = 0x7C00;

	.text : {
		*(.text*)
	}

	.rodata : {
		*(.rodata*)
	}

	.data : {
		*(.data*)
	}

	.bss : {
		*(.bss*)
	}

	. = 0x7DBE;
	.partition : {
		*(.partition*)
	}

	. = 0x7DFE;
	.signature : {
		SHORT(0xAA55);
	}
}