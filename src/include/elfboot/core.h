#ifndef __CORE_H__
#define __CORE_H__

#include <elfboot/types.h>
#include <elfboot/config.h>

#include <uapi/elfboot/errno.h>

#define min(x, y)			((x < y) ? x : y)
#define max(x, y)			((x > y) ? x : y)
#define abs(x)				((x > 0) ? x : -x)

#define abs_difference(x, y)		((x < y) ? y - x : x - y)

#define __round_mask(x, y)				\
	((__typeof__(x))((y) - 1))

#define round_up(x, y)					\
	((((x) - 1) | __round_mask(x, y)) + 1)

#define round_down(x, y)				\
	((x) & ~ __round_mask(x, y))

#define clamp(value, lower, upper)			\
	min((typeof(value))max(value, lower), upper)

#define ARRAY_SIZE(x)					\
	(sizeof(x) / sizeof(*(x)))

#endif /* __CORE_H__ */