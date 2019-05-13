#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

/*
 * Endianess
 */

static inline uint16_t swap_bytes16(uint16_t val)
{
	return (uint16_t)((val << 8) | (val >> 8));
}

static inline uint32_t swap_bytes32(uint32_t val)
{
	return ((val << 24)
		| ((val & (uint32_t)0x0000FF00UL) << 8)
		| ((val & (uint32_t)0x00FF0000UL) >> 8)
		| (val >> 24));
}

static inline uint64_t swap_bytes64(uint64_t val)
{
	return ((val << 56)
		| ((val & (uint64_t)0x000000000000FF00ULL) << 40)
		| ((val & (uint64_t)0x0000000000FF0000ULL) << 24)
		| ((val & (uint64_t)0x00000000FF000000ULL) <<  8)
		| ((val & (uint64_t)0x000000FF00000000ULL) >>  8)
		| ((val & (uint64_t)0x0000FF0000000000ULL) >> 24)
		| ((val & (uint64_t)0x00FF000000000000ULL) >> 40)
		| (val >> 56));
}

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

#define cputole16(x)                              ((uint16_t)(x))
#define cputole32(x)                              ((uint32_t)(x))
#define cputole64(x)                              ((uint64_t)(x))
#define letocpu16(x)                              ((uint16_t)(x))
#define letocpu32(x)                              ((uint32_t)(x))
#define letocpu64(x)                              ((uint64_t)(x))

#define cputobe16(x)                              swap_bytes16(x)
#define cputobe32(x)                              swap_bytes32(x)
#define cputobe64(x)                              swap_bytes64(x)
#define betocpu16(x)                              swap_bytes16(x)
#define betocpu32(x)                              swap_bytes32(x)
#define betocpu64(x)                              swap_bytes64(x)

#else /* __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__*/

#define cputole16(x)                              swap_bytes16(x)
#define cputole32(x)                              swap_bytes32(x)
#define cputole64(x)                              swap_bytes64(x)
#define letocpu16(x)                              swap_bytes16(x)
#define letocpu32(x)                              swap_bytes32(x)
#define letocpu64(x)                              swap_bytes64(x)

#define cputobe16(x)                              ((uint16_t)(x))
#define cputobe32(x)                              ((uint32_t)(x))
#define cputobe64(x)                              ((uint64_t)(x))
#define betocpu16(x)                              ((uint16_t)(x))
#define betocpu32(x)                              ((uint32_t)(x))
#define betocpu64(x)                              ((uint64_t)(x))

#endif /* __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__ */

#endif /* __TYPES_H__ */