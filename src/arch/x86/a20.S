#include <asm/linkage.h>

.code16

.section .text

# enable_a20:
#
# Tries to enable the A20 line in order to 
# support addressing even megabytes. To avoid 
# complications, the function tries to enable 
# the A20 line in three different ways:
#
#		- Enabling the line via BIOS interrupt
#		- Enabling the line using the fast A20 gate
#		- Enabling the line with the keyboard controller

GLOBAL(enable_a20)
	call    a20_check
	jnc     .a20_enabled

	call    enable_a20_bios
	call    a20_check
	jnc     .a20_enabled

	call    enable_a20_fast
	jnc     .a20_enabled

	call    enable_a20_8042
	call    a20_check

.a20_enabled:

	ret

enable_a20_bios:
	movw    $0x2401, %ax
	int     $0x15

	ret

enable_a20_fast:
	inb     $0x92, %al
	test    $2, %al
	jnz     .a20_fast_enabled

	orb     $2, %al
	andb    $0xfe, %al
	outb    %al, $0x92

.a20_fast_enabled:

	ret

enable_a20_8042:
	call    .a20_wait
	movb    $0xd1, %al
	outb    %al, $0x64

	call    .a20_wait
	movb    $0xdf, %al
	outb    %al, $0x60

	call    .a20_wait
	movb    $0xff, %al
	outb    %al, $0x64

	call    .a20_wait

	ret

.a20_wait:
	outb    %al, $0x80
	inb     $0x64, %al
	test    $1, %al
	jne     .a20_no_output

	outb    %al, $0x80
	inb     $0x60, %al
	jmp     .a20_wait

.a20_no_output:
	test    $2, %al
	jne     .a20_wait

	ret

a20_check:
	push    %ds
	push    %es
	xorw    %ax, %ax

	movw    %ax, %ds
	movw    $0xffff, %ax
	movw    %ax, %es
	movw    0x1000(,1), %ax
	movw    $5, %cx

.a20_check_loop:
	outb    %al, $0x80
	movw    0x1000(,1), %bx
	cmpw    %es:0x1010, %bx
	jne     .a20_8042_enabled

	inc     %bx

	movw    %bx, 0x1000(,1)
	cmpw    %es:0x1010, %bx
	jne     .a20_8042_enabled

	dec     %cx
	jnz     .a20_check_loop
	stc
	jmp     .a20_8042_return

.a20_8042_enabled:
	clc

.a20_8042_return:
	movw    %ax, 0x1000(,1)
	pop     %es
	pop     %ds

	ret
ENDPROC(enable_a20)
