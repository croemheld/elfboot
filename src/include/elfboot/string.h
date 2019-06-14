#ifndef __ELFBOOT_STRING_H__
#define __ELFBOOT_STRING_H__

#include <elfboot/core.h>

/*
 * Number utility functions
 */

static inline int is_digit(int ch)
{
	return (ch >= '0') && (ch <= '9');
}

static inline int is_xdigit(int ch)
{
	if (is_digit(ch))
		return true;

	if ((ch >= 'a') && (ch <= 'f'))
		return true;

	return (ch >= 'A') && (ch <= 'F');
}

void *memset(void *dst, int c, size_t len);

void *memcpy(void *dst, const void *src, size_t len);

void *memmove(void *dst, const void *src, size_t len);

int memcmp(const void *str1, const void *str2, size_t len);

int strcmp(const char *str1, const char *str2);

int strncmp(const char *str1, const char *str2, size_t count);

size_t strnlen(const char *str, size_t maxlen);

size_t strlen(const char *str);

char *strstr(const char *str1, const char *str2);

char *strchr(const char *str, int c);

size_t strspn(const char *str1, const char *str2);

size_t strcspn(const char *str1, const char *str2);

char *strtok(char *str, const char *delim);

uint32_t simple_strtoull(const char *cp, char **endp, unsigned int base);

int simple_strtol(const char *cp, char **endp, unsigned int base);

#endif /* __ELFBOOT_STRING_H__ */