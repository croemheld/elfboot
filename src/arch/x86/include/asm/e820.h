#ifndef __X86_E820_H__
#define __X86_E820_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include <asm/boot.h>
#include <asm/bda.h>
#include <asm/printf.h>

#include <elfboot/core.h>
#include <elfboot/string.h>
#include <elfboot/mm.h>

#include <uapi/asm/bootparam.h>

void bootmem_reserve(uint64_t addr, uint64_t size);

void bootmem_release(uint64_t addr, uint64_t size);

void bootmem_init(struct boot_params *boot_params);

#endif /* __X86_E820_H__ */