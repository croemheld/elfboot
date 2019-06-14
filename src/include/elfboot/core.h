#ifndef __CORE_H__
#define __CORE_H__

#include <elfboot/types.h>

#include <uapi/elfboot/errno.h>

#define min(x, y)                                 ((x < y) ? x : y)
#define max(x, y)                                 ((x > y) ? x : y)

#define ARRAY_SIZE(x)                             (sizeof(x) / sizeof(*(x)))

#endif /* __CORE_H__ */