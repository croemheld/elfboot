#ifndef __X86_DESC_H__
#define __X86_DESC_H__

#include <asm/linkage.h>

#define GATE_INTR	0x0E
#define GATE_TRAP	0x0F

struct gate_desc {
	uint16_t offset_lower;
	uint16_t segment;
	uint8_t __reserved;
	uint8_t type_attribute;
	uint16_t offset_upper;
} __packed;

struct desc_ptr {
	uint16_t limit;
	uint32_t offset;
} __packed;

#endif /* __X86_DESC_H__ */