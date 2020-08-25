#ifndef __ELFBOOT_TIME_H__
#define __ELFBOOT_TIME_H__

#include <elfboot/core.h>
#include <elfboot/linkage.h>

/*
 * We only directly work with milliseconds as the time unit. Use the following
 * macros to convert other time units to milliseconds or back.
 */

#define TIME_SECSTOMILLI(x)			(x) * 1000
#define TIME_MILLITOSECS(x)			(x) / 1000

#endif /* __ELFBOOT_TIME_H__ */