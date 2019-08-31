#ifndef __UAPI_ELFBOOT_DEBUG_H__
#define __UAPI_ELFBOOT_DEBUG_H__

static inline void bochs_bp(void)
{
	asm volatile("xchg %bx, %bx");
}

#endif /* __UAPI_ELFBOOT_DEBUG_H__ */