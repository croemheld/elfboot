#ifndef __BOOT_STRING_H__
#define __BOOT_STRING_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

void *memcpy(void *dst, const void *src, size_t len);

void *memset(void *dst, int c, size_t len);

void *segcpy(uint16_t dseg, uint16_t doff, uint16_t sseg, uint16_t soff, size_t len);

void farcpy(void *dst, const void *src, size_t len);

int memcmp(const void *s1, const void *s2, size_t len);

int strcmp(const char *str1, const char *str2);

int strncmp(const char *str1, const char *str2, size_t count);

size_t strnlen(const char *str, size_t maxlen);

size_t strlen(const char *str);

char *strstr(const char *str1, const char *str2);

char *strchr(const char *str, int c);

uint32_t simple_strtoull(const char *cp, char **endp, unsigned int base);

int simple_strtol(const char *cp, char **endp, unsigned int base);

#endif /* __BOOT_STRING_H__ */