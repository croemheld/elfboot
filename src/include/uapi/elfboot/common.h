#ifndef __UAPI_COMMON_H__
#define __UAPI_COMMON_H__

#include <elfboot/core.h>

#define container_of(ptr, type, member)					\
    ({									\
        const typeof(((type *)0)->member) *__mptr = (ptr);		\
        (type *)((char *)__mptr - offsetof(type, member));		\
    })

#define array_for_each(pos, array, count, index)			\
	for ((index) = 0, (pos) = (array)[index]; 			\
	     (index) < (count); 					\
	     (index)++, (pos) = (array[index]))

#define array_for_each_entry(pos, array, count)				\
	for ((pos) = (array); (pos) < (array) + count; (pos)++)

#define array_for_each_entry_callback(pos, array, count, callback) do {	\
	array_for_each_entry(pos, array, count)				\
		(callback)((pos));					\
} while (0)

/*
 * Alignment for power of 2 bytes
 */

#define ALIGN(x, a)			((x) & ~(a - 1))
#define BOUNDARY(x, a)			((x) &  (a - 1))

#define ALIGNED(x, a)			(BOUNDARY(x, a) == 0)

/*
 * Conversions
 */

static inline void *uinttvptr(uint32_t val)
{
	return (void *)val;
}

static inline uint32_t vptrtuint(void *ptr)
{
	return (uint32_t)ptr;
}

static inline void *vptradd(void *ptr, uint32_t offset)
{
	return ptr + offset;
}

static inline uint32_t segment_offset_addr(uint16_t segment, uint16_t offset)
{
	return (segment << 4) + offset;
}

static inline void *segment_offset_ptr(uint32_t ptr)
{
	return uinttvptr(segment_offset_addr(ptr >> 16, ptr & 0xffff));
}

static inline uint8_t u8val(void *ptr)
{
	return *((uint8_t *)ptr);
}

static inline uint16_t u16val(void *ptr)
{
	return *((uint16_t *)ptr);
}

static inline uint32_t u32val(void *ptr)
{
	return *((uint32_t *)ptr);
}

#endif /* __UAPI_COMMON_H__ */