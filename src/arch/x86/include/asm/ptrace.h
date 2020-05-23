#ifndef __X86_PTRACE_H__
#define __X86_PTRACE_H__

#include <elfboot/core.h>
#include <elfboot/linkage.h>

#include <asm/traps.h>

#define PTRACE_MAX_FRAMES	10

void __dump_stack(uint32_t eip, uint32_t ebp);

void dump_stack(void);

#endif /* __X86_PTRACE_H__ */