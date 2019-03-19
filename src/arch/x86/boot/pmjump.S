#include <boot/boot.h>
#include <boot/segment.h>
#include <boot/linkage.h>

.section .data, "aw", @progbits

	.global boot_gdt
	.align 0x10

boot_gdt:
	
	#
	# Null segment
	#

	GDT_SEG_NULL

	#
	# 32-bit code and data segment
	#

	GDT_SEG(0, 0xFFFFFFFF, 0x9A, 0xCF)
	GDT_SEG(0, 0xFFFFFFFF, 0x92, 0xCF)

	#
	# 16-bit code and data segment
	#

	GDT_SEG(0, 0x0000FFFF, 0x9A, 0x00)
	GDT_SEG(0, 0x0000FFFF, 0x92, 0x00)
boot_gdt_end:

	.global boot_gdt_desc
	.align 0x10

boot_gdt_desc:
	.word (boot_gdt_end - boot_gdt - 1)
	.long (boot_gdt)
