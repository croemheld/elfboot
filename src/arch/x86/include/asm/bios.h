#ifndef __X86_BIOS_H__
#define __X86_BIOS_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include <boot/regs.h>

void bioscall(uint8_t int_no, struct biosregs *ireg, struct biosregs *oreg);

#endif /* __X86_BIOS_H__ */