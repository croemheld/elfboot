#ifndef __ELFBOOT_E820_H__
#define __ELFBOOT_E820_H__

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

#define E820_MEMORY_TYPE_INVALID                   0
#define E820_MEMORY_TYPE_AVAILABLE                 1
#define E820_MEMORY_TYPE_RESERVED                  2
#define E820_MEMORY_TYPE_ACPI_RECLAIMABLE          3
#define E820_MEMORY_TYPE_ACPI_NVS                  4
#define E820_MEMORY_TYPE_BAD_MEMORY                5

#endif /* __ELFBOOT_E820_H__ */