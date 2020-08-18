#ifndef __X86_SEGMENT_H__
#define __X86_SEGMENT_H__

#define GDT_NULL		0x00

#define GDT_CODE16		0x08
#define GDT_DATA16		0x10

#define GDT_CODE32		0x18
#define GDT_DATA32		0x20

#ifdef __ASSEMBLER__

#define GDT_SEG_NULL					\
	.quad (0)

#define GDT_SEG(base, limit, access, flags)		\
	.word ((limit) & 0xFFFF), ((base) & 0xFFFF);	\
	.byte (((base) >> 16) & 0xFF), (access),	\
		(flags), (((base) >> 24) & 0xFF)

#else /* !__ASSEMBLER__ */

#define RM_SEG(x)		((x) & 0x000ffff0) >> 4
#define RM_OFF(x)		((x) & 0x0000000f) >> 0

void kernel_realmode_jump(uint16_t rm_seg, uint16_t kn_seg);

#endif /* __ASSEMBLER__ */

#endif /* __X86_SEGMENT_H__ */