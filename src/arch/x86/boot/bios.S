#include <boot/bios.h>
#include <boot/boot.h>
#include <boot/linkage.h>

.code16

.section .text

GLOBAL(bioscall)
	cmpb    %al, 2f
	je      1f
	movb    %al, 2f
	jmp     1f
1:
	pushfl
	pushw   %fs
	pushw   %gs
	pushal

	subw    $44, %sp
	movw    %dx, %si
	movw    %sp, %di
	movw    $11, %cx
	rep movsd

	popal
	popw    %gs
	popw    %fs
	popw    %es
	popw    %ds
	popfl

	# INT instruction in byte code

	.byte   0xcd
2:
	.byte   0x00

	pushfl
	pushw   %ds
	pushw   %es
	pushw   %fs
	pushw   %gs
	pushal

	cld
	movzwl  %sp, %esp
	movw    %cs, %ax
	movw    %ax, %ds
	movw    %ax, %es

	movw    68(%esp), %di
	andw    %di, %di
	jz      3f
	movw    %sp, %si
	movw    $11, %cx
	rep movsd

3:	addw    $44, %sp

	popal
	popw    %gs
	popw    %fs
	popfl
	
	retl
ENDPROC(bioscall)
