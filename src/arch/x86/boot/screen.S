#include <boot/boot.h>
#include <boot/linkage.h>

/*
 * TODO CRO: Relocate in C file
 */

.code16

.section .text

# clear_sceen:
#
# Clears the screen on startup.

GLOBAL(clear_screen)
	push    %bp
	movw    %sp, %bp

	pusha

	movb    $0x07, %ah
	movb    $0x00, %al
	movb    $0x07, %bh
	movw    $0x00, %cx
	movb    $0x18, %dh
	movb    $0x4f, %dl

	int     $0x10

	# Reset the text mode cursor

	call    reset_cursor

	popa

	movw    %bp, %sp
	pop     %bp

	ret
ENDPROC(clear_screen)

# reset_cursor:
#
# Resets the cursor to point to the first 
# character in the framebuffer.

GLOBAL(reset_cursor)
	push    %bp
	movw    %sp, %bp

	pusha

	movb    $0x00, %dh
	movb    $0x00, %dl
	movb    $0x02, %ah
	movb    $0x00, %bh

	int     $0x10

	popa

	movw    %bp, %sp
	pop     %bp

	ret
ENDPROC(reset_cursor)
