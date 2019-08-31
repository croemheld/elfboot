#ifndef __BOOT_CMDLINE_H__
#define __BOOT_CMDLINE_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

bool cmdline_get_boolean_value(const char *key);

bool cmdline_get_boolean_default_value(const char *key, bool default_value);

uint32_t cmdline_get_int_value(const char *key);

uint32_t cmdline_get_int_default_value(const char *key, uint32_t default_value);

char *cmdline_get_string_value(const char *key);

char *cmdline_get_string_default_value(const char *key, const char *default_value);

#endif /* __BOOT_CMDLINE_H__ */