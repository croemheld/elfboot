#include <asm/boot.h>
#include <asm/segment.h>
#include <asm/linkage.h>

.code16

.section .rodata, "a", @progbits

LOCAL(bios_error_msg)
	.asciz "BIOS error\r\n"

#
# This section contains the boot sector. We place it at the dedicated .boot 
# section of the binary whereas all other code is located after the first 
# 512 bytes.
#

.section .text

#
# Bootstrap entry point
#

GLOBAL(_arch_start)
	ljmp $0, $_arch_start16

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

#
# The actual code after clearing %cs
#
LOCAL(_arch_start16)
 	xorw    %ax, %ax
 	movw    %ax, %ds
 	movw    %ax, %es
 	movw    %ax, %fs
 	movw    %ax, %gs
 	movw    %ax, %ss

 	popl    %eax
 
 	movw    $BOOT_STACK_START, %sp

	pushl   %eax

	# Enable A20 line

	call    enable_a20
	jc      .bios_error

	# Prepare to jump into unreal mode

	call    pmmode_jmp

.code32

	# Clear the EBP register for a clean stack trace

	movl    $0, %ebp

	# Finally, calllbootloader main routine

	pushl   %edx
	calll   arch_setup

	# Halt the machine

	cli

LOCAL(.die)
	hlt

#
# Endless loop when failing
#

	jmp     .die
ENDPROC(_arch_start16)
