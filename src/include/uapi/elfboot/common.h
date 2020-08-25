#ifndef __UAPI_ELFBOOT_COMMON_H__
#define __UAPI_ELFBOOT_COMMON_H__

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

/*
 * tvptr:
 *
 * Cast a 32-bit unsigned value to a pointer.
 */
#define tvptr(val)						\
	((void *)(val))

/*
 * tuint:
 *
 * Cast a pointer to a 32-bit unsigned value.
 */
#define tuint(ptr)						\
	((uint32_t)(ptr))

/*
 * vptradd:
 *
 * Get a pointer to address ptr + offset, ignoring 
 * the type of the object residing at address ptr.
 */
#define vptradd(ptr, offset)					\
	tvptr(tuint((ptr)) + (offset))

/*
 * fptr_val:
 *
 * Get the address of a real mode segment-offset address.
 */
#define fptr_val(seg, off)				\
	((seg << 4) + (off))

/*
 * fptr:
 *
 * Get a pointer to a real mode segment-offset address.
 */
#define fptr(ptr)					\
	tvptr(fptr_val((ptr) >> 16, (ptr) & 0xffff))

#endif /* __UAPI_ELFBOOT_COMMON_H__ */