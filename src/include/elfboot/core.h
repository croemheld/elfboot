#ifndef __CORE_H__
#define __CORE_H__

#include <elfboot/types.h>

#include <uapi/elfboot/errno.h>

#define min(x, y)                         ((x < y) ? x : y)
#define max(x, y)                         ((x > y) ? x : y)

#define __round_mask(x, y)                ((__typeof__(x))((y) - 1))
#define round_up(x, y)                    ((((x) - 1) | __round_mask(x, y)) + 1)
#define round_down(x, y)                  ((x) & ~__round_mask(x, y))

#define ARRAY_SIZE(x)                     (sizeof(x) / sizeof(*(x)))

#endif /* __CORE_H__ */