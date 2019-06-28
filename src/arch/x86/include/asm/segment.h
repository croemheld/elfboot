#ifndef __X86_SEGMENT_H__
#define __X86_SEGMENT_H__

#define BOOT_GDT_NULL		0x00

#define BOOT_GDT_CODE16		0x08
#define BOOT_GDT_DATA16		0x10

#define BOOT_GDT_CODE32		0x18
#define BOOT_GDT_DATA32		0x20

#ifdef __ASSEMBLER__

#define GDT_SEG_NULL					\
	.quad (0)

#define GDT_SEG(base, limit, access, flags)		\
	.word ((limit) & 0xFFFF), ((base) & 0xFFFF);	\
	.byte (((base) >> 16) & 0xFF), (access),	\
		(flags), (((base) >> 24) & 0xFF)

#endif /* __ASSEMBLER__ */

#endif /* __X86_SEGMENT_H__ */