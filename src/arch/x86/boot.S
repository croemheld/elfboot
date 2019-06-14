#include <asm/boot.h>
#include <asm/segment.h>
#include <asm/linkage.h>

.code16

.section .rodata, "a", @progbits

bios_error_msg:
	.asciz "BIOS function error\r\n"

a20_error_msg:
	.asciz "Could not enable A20 line\r\n"

#
# This section contains the boot sector. We place it at the dedicated .boot 
# section of the binary whereas all other code is located after the first 
# 512 bytes.
#

.section .boot, "aw", @progbits

	.global _start

#
# Bootloader entry point
#

_start:
	ljmp $0, $_setup16

# .bios_print
#
# A low-level printf function which utilizes the
# BIOS function int 13h.

LOCAL(.bios_print)
	xorw    %ax, %ax

.print_char:
	lodsb

	test    %al, %al
	jz      .print_end

	movb    $0x0e, %ah
	int     $0x10

	jmp     .print_char

.print_end:

	ret
ENDPROC(.bios_print)

# .a20_error:
#
# Prints an error message to indicate the lack 
# of A20 support before halting the system.

LOCAL(.a20_error)
	movw    $a20_error_msg, %si
	call    .bios_print

	jmp     .
ENDPROC(.a20_error)

# .bios_error
#
# Halts the system and puts an error message on
# the screen before entering an infinite loop.

LOCAL(.bios_error)

	# Clear modifications made to the registers

	xorw    %ax, %ax
	movw    %ax, %ds

	movw    $bios_error_msg, %si
	call    .bios_print

	jmp     .
ENDPROC(.bios_error)

#
# The actual code after clearing %cs
#

_setup16:
	xorw    %ax, %ax
	movw    %ax, %ds
	movw    %ax, %es
	movw    %ax, %fs
	movw    %ax, %gs
	movw    %ax, %ss

	movw    $BOOT_STACK_ADDR, %sp

	# Enable A20 line

	call    enable_a20
	jc      .bios_error

	# Prepare to jump into unreal mode

	lgdt    boot_gdt_desc

	cli

	movl    %cr0, %eax
	orl     $0x1, %eax
	movl    %eax, %cr0

	jmpl    $BOOT_GDT_CODE16, $.protcseg

.protcseg:
	movw    $BOOT_GDT_DATA16, %ax
	movw    %ax, %ds
	movw    %ax, %es
	movw    %ax, %fs
	movw    %ax, %gs
	movw    %ax, %ss

	movl    %cr0,  %eax
	andl    $(~1), %eax
	movl    %eax,  %cr0

	jmpl    $0, $.unrealcseg

.unrealcseg:
	xorw    %ax, %ax
	movw    %ax, %ds
	movw    %ax, %es
	movw    %ax, %fs
	movw    %ax, %gs
	movw    %ax, %ss

	sti

	# Finally, call bootloader main routine

	movb    %dl, %al
	call    main

	jmp     .
