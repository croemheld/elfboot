#ifndef __X86_MEMORY_H__
#define __X86_MEMORY_H__

#include <elfboot/core.h>
#include <elfboot/linkage.h>

uint32_t arch_memory_lower_size(void);

uint32_t arch_memory_upper_size(void);

#endif /* __X86_MEMORY_H__ */