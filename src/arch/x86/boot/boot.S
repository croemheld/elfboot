#include <asm/boot.h>
#include <asm/segment.h>
#include <asm/linkage.h>

.code16

.section .text

#
# Bootloader entry point
#

GLOBAL(_start)
	ljmp    $0, $_start16

# Stub for boot information table
#
# The boot information table is placed at offset
# 8 of the boot image and occupies the memory to
# offset 63 (including).

.org 8

# Boot information table
#
# Filled by xorriso option -boot-info-table

bootimage_bit:
bootimage_pvd:
	.long 0
bootimage_lba:
	.long 0
bootimage_len:
	.long 0
bootimage_checksum:
	.long 0
bootimage_reserved:
	.space (10 * 4)

# Boot information table (custom)
#
# We create out own boot information table for the next stage so
# that we can effortlessly lad the final stage of the Bootloader
# into memory.

elfboot_bit:
elfboot_bin_pvd:
	.long 0
elfboot_bin_lba:
	.long 0

# elfboot_bin_len: Length of elfboot (stage 2) image
#
# Filled in by elfconf -f bootimg.elf -s elfboot_bin_len -v <length>

GLOBAL(elfboot_bin_len)
	.long 0
ENDSYM(elfboot_bin_len)

bootuid_bfd_lba:
	.long 0

# bootuid_bfd_len: Boot identification file
#
# Filled in by elfconf -f bootimg.elf -s bootuid_bfd_len -v <length>

GLOBAL(bootuid_bfd_len)
	.long 0
ENDSYM(bootuid_bfd_len)

elfboot_reserved:
	.space (8 * 4)

# .bios_print
#
# A low-level printf function which utilizes the
# BIOS function int 13h.
#
# Parameters:
#
#	- %ax: Message to print

LOCAL(.bios_print)
	movw    %ax, %si

.bios_print_char:
	lodsb

	test    %al, %al
	jz      .bios_print_end

	movb    $0x0e, %ah
	int     $0x10

	jmp     .bios_print_char

.bios_print_end:

	ret
ENDPROC(.bios_print)

# .bios_error
#
# Halts the system and puts an error message on
# the screen before entering an infinite loop.

LOCAL(.bios_error)
	movw    $bios_error_msg, %ax
	call    .bios_print

	jmp     .die
ENDPROC(.bios_error)

# load_elfboot
#
# Loads the next stage into memory. The function uses the values
# from bootimage_lba and elfboot_len for the starting LBA and
# the elfboot image and the length of it.

LOCAL(load_elfboot)

	# Update elfboot boot info table

	movl    bootimage_pvd, %eax
	movl    %eax, elfboot_bin_pvd

	# Get start and length of elfboot second stage image

	movl    bootimage_lba, %eax
	addl    $1, %eax
	movl    %eax, elfboot_bin_lba

	movl    elfboot_bin_len, %ecx
	movw    $SECOND_STAGE_SEGMENT, %bx

	# The length of the elfboot image is given in bytes

	decw    %cx
	shrw    $11, %cx
	incw    %cx

	movw    $dap_elfboot, %si
	movw    %ax, 0x8(%si)
	movw    %cx, 0x2(%si)
	movw    %bx, 0x6(%si)

	movb    $0x42, %ah
	int     $0x13
	jc      .bios_error

	ret
ENDPROC(load_elfboot)

#
# The actual code after clearing %cs
#

LOCAL(_start16)
	cli

	xorw    %ax, %ax
	movw    %ax, %ss
	movw    %ax, %ds
	movw    %ax, %es

	movw    $BOOT_STACK_START, %sp

	call    load_elfboot

	# Pass address of boot information table to elfboot image

	pushl   $elfboot_bit

	ljmp    $0, $SECOND_STAGE_ADDRESS
ENDPROC(_start16)

#
# Endless loop when failing
#

LOCAL(.die)
	jmp     .die
ENDPROC(.die)

.section .data

GLOBAL(dap_elfboot)
	.word 0x0010
dap_elfboot_sectors:
	.word 0
dap_elfboot_segment:
	.long 0
dap_elfboot_lower_lba:
	.long 0
dap_elfboot_upper_lba:
	.long 0
ENDSYM(dap_elfboot)

.section .rodata, "a", @progbits

LOCAL(bios_error_msg)
	.asciz "BOOTIMAGE: BIOS error\r\n"

#
# Boot partition table entries
#

.section .partition, "a", @progbits

.org 0
