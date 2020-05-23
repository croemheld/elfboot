#ifndef __X86_DESC_H__
#define __X86_DESC_H__

#include <asm/linkage.h>

#define GATE_INTR	0x0E

struct gate_desc {
	uint16_t offset_lower;
	uint16_t segment;
	uint8_t __reserved;
	uint8_t type_attribute;
	uint16_t offset_upper;
} __packed;

struct desc_ptr {
	uint16_t size;
	uint16_t addr_lower;
	uint16_t addr_upper;
};

#endif /* __X86_DESC_H__ */