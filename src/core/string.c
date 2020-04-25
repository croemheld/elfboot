#include <elfboot/core.h>
#include <elfboot/string.h>

int memcmp(const void *p1, const void *p2, size_t n)
{
	size_t i;
	const uint8_t *a = p1;
	const uint8_t *b = p2;

	for (i = 0; i < n; i++) {
		if (a[i] < b[i]) 
			return -1;

		if (a[i] > b[i])
			return 1;
	}

	return 0;
}

int strcmp(const char *str1, const char *str2)
{
	const char *s1 = str1;
	const char *s2 = str2;

	int delta = 0;

	while (*s1 || *s2) {
		delta = *s1 - *s2;

		if (delta)
			return delta;

		s1++;
		s2++;
	}

	return 0;
}

int strncmp(const char *str1, const char *str2, size_t count)
{
	unsigned char c1, c2;

	while (count) {
		c1 = *str1++;
		c2 = *str2++;

		if (c1 != c2)
			return c1 < c2 ? -1 : 1;

		if (!c1)
			break;

		count--;
	}

	return 0;
}

size_t strnlen(const char *str, size_t maxlen)
{
	const char *es = str;

	while (*es && maxlen) {
		es++;
		maxlen--;
	}

	return (es - str);
}

size_t strlen(const char *str)
{
	const char *sc;

	for (sc = str; *sc != '\0'; ++sc);

	return sc - str;
}

char *strcpy(char *dst, const char *str)
{
	return memcpy(dst, str, strlen(str));
}

char *strncpy(char *dst, const char *str, size_t len)
{
	char *ret = dst;

	if (len >= strlen(str))
		return strcpy(dst, str);

	while (len--)
		*dst++ = *str++;

	return ret;
}

char *strstr(const char *str1, const char *str2)
{
	size_t len1, len2;

	len2 = strlen(str2);

	if (!len2)
		return (char *)str1;

	len1 = strlen(str1);

	while (len1 >= len2) {
		len1--;

		if (!memcmp(str1, str2, len2))
			return (char *)str1;

		str1++;
	}

	return NULL;
}

char *strchr(const char *str, int c)
{
	while (*str != (char)c) {
		if (*str++ == '\0')
			return NULL;
	}

	return (char *)str;
}

char *strrchr(const char *str, int c)
{
	char *ret = (char *)str + strlen(str) - 1;

	while (*ret != (char)c) {
		if (ret-- == str)
			return NULL;
	}

	return ret;
}

size_t strspn(const char *str1, const char *str2)
{
	size_t ret = 0;

	while (*str1 && strchr(str2, *str1++))
		ret++;

	return ret;    
}

size_t strcspn(const char *str1, const char *str2)
{
	size_t ret = 0;

	while (*str1)
		if (strchr(str2, *str1))
			return ret;
        	else
			str1++, ret++;

	return ret;
}


char *strtok(char *str, const char *delim)
{
	static char* p = 0;

	if (str)
		p = str;
	else if (!p)
		return 0;

	str = p + strspn(p, delim);
	p = str + strcspn(str, delim);

	if (p == str)
		return p = 0;

	p = *p ? *p = 0, p + 1 : 0;

	return str;
}

#define TOLOWER(x) ((x) | 0x20)

/*
 * TODO CRO: Merge with base "guesser" from core/printf.c.
 */
static unsigned int simple_guess_base(const char *cp)
{
	if (cp[0] == '0') {
		if (TOLOWER(cp[1]) == 'x' && is_xdigit(cp[2]))
			return 16;
		else
			return 8;
	} else {
		return 10;
	}
}

unsigned int strtoul(const char *cp, char **endp, unsigned int base)
{
	unsigned int result = 0;

	if (!base)
		base = simple_guess_base(cp);

	if (base == 16 && cp[0] == '0' && TOLOWER(cp[1]) == 'x')
		cp += 2;

	while (is_xdigit(*cp)) {
		unsigned int value;

		value = is_digit(*cp) ? *cp - '0' : TOLOWER(*cp) - 'a' + 10;

		if (value >= base)
			break;

		result = result * base + value;
		cp++;
	}

	if (endp)
		*endp = (char *)cp;

	return result;
}

int strtol(const char *cp, char **endp, unsigned int base)
{
	if (*cp == '-')
		return -strtoul(cp + 1, endp, base);

	return strtoul(cp, endp, base);
}