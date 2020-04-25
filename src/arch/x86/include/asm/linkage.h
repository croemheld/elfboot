#ifndef __X86_LINKAGE_H__
#define __X86_LINKAGE_H__

#ifdef __ASSEMBLER__

#define LOCAL(name)				\
name:

#define GLOBAL(name)			\
    .global name;				\
name:

#define ENDPROC(name)			\
    .type name, @function;		\
    .size name, .-name

#define ENDSYM(name)			\
    .size name, .-name

#endif /* __ASSEMBLER__ */

#endif /* __X86_LINKAGE_H__ */