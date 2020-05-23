#ifndef __X86_LINKAGE_H__
#define __X86_LINKAGE_H__

#ifdef __ASSEMBLER__

#define SYM_L_GLOBAL(name)	.global name
#define SYM_L_LOCAL(name)	/* Nothing */

#define ALIGN(align)		.p2align (align), 0x90

#define SYMBOL(name, linkage, align)	\
	ALIGN(align);						\
	linkage(name);						\
name:

#define LOCAL(name)		SYMBOL(name, SYM_L_LOCAL,  0)

#define GLOBAL(name)	SYMBOL(name, SYM_L_GLOBAL, 0)

#define ENDPROC(name)			\
    .type name, @function;		\
    .size name, .-name

#define ENDSYM(name)			\
    .size name, .-name

#endif /* __ASSEMBLER__ */

#endif /* __X86_LINKAGE_H__ */