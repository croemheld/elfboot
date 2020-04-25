#include <asm/boot.h>
#include <asm/linkage.h>

.code32

.section .text

#
# memcpy
#

GLOBAL(memcpy)
	pushl   %esi
	pushl   %edi

	movl    0x0C(%esp), %edi
	movl    0x10(%esp), %esi
	movl    0x14(%esp), %ecx
	pushl   %ecx
	shrl    $2, %ecx
	rep movsl

	popl    %ecx
	andl    $3, %ecx
	rep movsb

	popl    %edi
	popl    %esi

	retl
ENDPROC(memcpy)

#
# memset (for 8-, 16- and 32-bit values)
#

GLOBAL(memset)
	pushl   %edi

	movl    0x08(%esp), %edi
	movzbl  0x0C(%esp), %eax

	imull   $0x01010101,%eax
	movl    0x10(%esp), %ecx
	pushl   %ecx
	shrl    $2, %ecx
	rep stosl

	popl    %ecx
	andl    $3, %ecx
	rep stosb

	popl    %edi

	retl
ENDPROC(memset)

GLOBAL(memset16)
	pushl   %edi

	movl    0x08(%esp), %edi
	movzbl  0x0C(%esp), %eax

	imull   $0x00010001,%eax
	movl    0x10(%esp), %ecx
	pushl   %ecx
	shrl    $2, %ecx
	rep stosl

	popl    %ecx
	andl    $3, %ecx
	rep stosb

	popl    %edi

	retl
ENDPROC(memset16)

GLOBAL(memset32)
	pushl   %edi

	movl    0x08(%esp), %edi
	movzbl  0x0C(%esp), %eax

	movl    0x10(%esp), %ecx
	pushl   %ecx
	shrl    $2, %ecx
	rep stosl

	popl    %ecx
	andl    $3, %ecx
	rep stosb

	popl    %edi

	retl
ENDPROC(memset32)

#
# memmove
#

GLOBAL(memmove)
	pushl   %esi
	pushl   %edi

	movl    0x0C(%esp), %edi
	movl    0x10(%esp), %esi
	movl    0x14(%esp), %ecx

	cmpl    %esi, %edi
	jb      .memmove_continue

	addl    %ecx, %edi
	addl    %ecx, %esi

	cmpl    $4, %ecx
	jge     .memmove_rvlength

	decl    %edi
	decl    %esi

	jmp     .memmove_std

.memmove_rvlength:

	subl    $4, %edi
	subl    $4, %esi

.memmove_std:

	std

.memmove_continue:

	pushl   %ecx
	shrl    $2, %ecx
	rep movsl

	popl    %ecx
	andl    $3, %ecx
	rep movsb

	cld

	popl    %edi
	popl    %esi

	retl
ENDPROC(memmove)
