#include <asm/linkage.h>

.code16

.section .text

GLOBAL(memcpy)
	pushl   %esi
	pushl   %edi

	movl    %eax, %edi
	movl    %edx, %esi
	pushl   %ecx
	shrl    $2, %ecx
	rep; movsl

	popl    %ecx
	andl    $3, %ecx
	rep; movsb

	popl    %edi
	popl    %esi

	retl
ENDPROC(memcpy)

GLOBAL(memset)
	pushl   %edi

	movl    %eax, %edi
	movzbl  %dl, %eax
	imull   $0x01010101, %eax
	pushl   %ecx
	shrl    $2, %ecx
	rep; stosl

	popl    %ecx
	andl    $3, %ecx
	rep; stosb

	popl    %edi

	retl
ENDPROC(memset)

GLOBAL(memset16)
	pushl   %edi

	movl    %eax, %edi
	movzwl  %dx, %eax
	imull   $0x00010001, %eax
	pushl   %ecx
	shrl    $2, %ecx
	rep; stosl

	popl    %ecx
	andl    $3, %ecx
	rep; stosb

	popl    %edi

	retl
ENDPROC(memset16)

GLOBAL(memset32)
	pushl   %edi

	movl    %eax, %edi
	movl    %edx, %eax
	imull   $0x00000001, %eax
	pushl   %ecx
	shrl    $2, %ecx
	rep; stosl

	popl    %ecx
	andl    $3, %ecx
	rep; stosb

	popl    %edi

	retl
ENDPROC(memset32)

GLOBAL(memmove)
	pushl   %esi
	pushl   %edi

	movl    %edx, %esi
	movl    %eax, %edi
	pushl   %ecx

	cmpl    %edx, %eax
	je      .memmove_done
	jb      .memmove_fast

.memmove_slow:

	std

	addl    %ecx, %esi
	subl    $4, %esi
	addl    %ecx, %edi
	subl    $4, %edi

	shrl    $2, %ecx
	rep; movsl

	popl    %ecx
	andl    $3, %ecx
	rep; movsb

	cld

	jmp     .memmove_done

.memmove_fast:
	shrl    $2, %ecx
	rep; movsl

	popl    %ecx
	andl    $3, %ecx
	rep; movsb

.memmove_done:
	popl    %edi
	popl    %esi

	retl
ENDPROC(memmove)
