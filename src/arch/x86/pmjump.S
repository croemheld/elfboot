#include <asm/segment.h>
#include <asm/linkage.h>

.section .data, "aw", @progbits

	.global boot_gdt
	.align 0x10

boot_gdt:
	
	#
	# Null segment
	#

	GDT_SEG_NULL

	#
	# Unreal mode code and data segment
	#

	GDT_SEG(0, 0xFFFF, 0x9A, 0x8F)
	GDT_SEG(0, 0xFFFF, 0x92, 0x8F)

	#
	# Protected mode code and data segment
	#

	GDT_SEG(0, 0xFFFF, 0x9A, 0xCF)
	GDT_SEG(0, 0xFFFF, 0x92, 0xCF)
boot_gdt_end:

	.global boot_gdt_desc
	.align 0x10

boot_gdt_desc:
	.word (boot_gdt_end - boot_gdt - 1)
	.long (boot_gdt)
