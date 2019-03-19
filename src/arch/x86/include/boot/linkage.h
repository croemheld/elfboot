#ifndef __X86_LINKAGE_H__
#define __X86_LINKAGE_H__

#ifdef __ASSEMBLER__

#define GLOBAL(fname)                                                          \
    .global fname;                                                             \
fname:

#define ENDPROC(fname)                                                         \
    .type fname, @function;                                                    \
    .size fname, .-fname

#endif /* __ASSEMBLER__ */

#endif /* __X86_LINKAGE_H__ */