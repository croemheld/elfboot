#ifndef __X86_MATH_H__
#define __X86_MATH_H__

#include <elfboot/core.h>
#include <elfboot/linkage.h>

#include <uapi/elfboot/const.h>

uint64_t arch_div(uint64_t val, uint32_t div, uint32_t *rem);

#endif /* __X86_MATH_H__ */