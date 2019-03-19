#include <boot/boot.h>
#include <boot/segment.h>
#include <boot/linkage.h>

.code16

/*
 * Real mode memcpy
 */

GLOBAL(memcpy)
	push    %si
	push    %di

	movw    %ax, %di
	movw    %dx, %si
	push    %cx
	shrw    $2, %cx
	rep movsl

	pop     %cx
	andw    $3, %cx
	rep movsb

	pop     %di
	pop     %si

	retl
ENDPROC(memcpy)

/*
 * Real mode memset
 */

GLOBAL(memset)
	push    %di

	movw    %ax, %di
	movzbl  %dl, %eax
	imull   $0x01010101, %eax
	push    %cx
	shrw    $2, %cx
	rep stosl

	pop     %cx
	andw    $3, %cx
	rep stosb

	pop     %di

	retl
ENDPROC(memset)

/*
 * memcpy variation to copy over the 64 KiB mark
 */

GLOBAL(farcpy)
	push    %si
	push    %di

	push    %eax
	push    %edx
	push    %ecx

 	# Prepare jump to protected mode

 	lgdt    boot_gdt_desc

 	movl    %cr0, %eax
 	orl     $0x1, %eax
 	movl    %eax, %cr0

 	# Jump into protected mode

 	ljmp    $BOOT_GDT_CODE32, $.farcpy_pm32

.code32

.farcpy_pm32:
	movw    $BOOT_GDT_DATA32, %ax
	movw    %ax, %ds
	movw    %ax, %es

	# Set parameters for copying

	pop     %ecx
	pop     %esi
	pop     %edi
	rep movsl

	movw    $BOOT_GDT_DATA16, %ax
	movw    %ax, %ds
	movw    %ax, %es
	ljmp    $BOOT_GDT_CODE16, $.farcpy_pm16

.code16

.farcpy_pm16:
	movl    %cr0, %eax
	andl    $(~1), %eax
	movl    %eax, %cr0

	# Prepare jump back into real mode

	xorw    %ax, %ax
	movw    %ax, %ds
	movw    %ax, %es

	# Jump into real mode

	ljmp    $0, $.farcpy_rm

.farcpy_rm:
	pop     %di
	pop     %si
	
	retl
ENDPROC(farcpy)