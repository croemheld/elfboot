#include <asm/boot.h>
#include <asm/linkage.h>

.code32

.section .real, "ax", @progbits

GLOBAL(kernel_realmode_jump)

	call    rmmode_jmp

.code16
	movw    0x4(%esp), %ax
	movw    0x8(%esp), %dx

	ljmp    $0, $1f

1:	movw    %ax, %ds
	movw    %ax, %es
	movw    %ax, %fs
	movw    %ax, %gs
	movw    %ax, %ss

	movw    $0x9800, %sp

	pushw   %dx
	pushw   $0

	lret
ENDPROC(kernel_realmode_jump)