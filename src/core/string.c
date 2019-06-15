#include <elfboot/core.h>
#include <elfboot/string.h>

void* memcpy(void *dst, const void *src, size_t n)
{
	unsigned char *d = (unsigned char *)dst;
	unsigned char *s = (unsigned char *)src;

	for(size_t i = 0; i < n; i++)
		d[i] = s[i];

	return dst;
}

void* memset(void *dst, int c, size_t len)
{
	unsigned char *d = (unsigned char *)dst;

	for(size_t i = 0; i < len; i++)
		d[i] = (unsigned char)c;

	return dst; 
}

void* memset16(void *dst, int c, size_t len)
{	
	size_t i;
	unsigned char *d = (unsigned char *)dst;

	for (i = 0; i < (len & (~1)); i+= 2)
		memcpy(d + i, &c, 2);

	for ( ; i < len; i++)
		d[i] = ((unsigned char *)&c)[i & 1];

	return dst;
}

void* memset32(void *dst, int c, size_t len)
{	
	size_t i;
	unsigned char *d = (unsigned char *)dst;

	for (i = 0; i < (len & (~3)); i+= 4)
		memcpy(d + i, &c, 4);

	for ( ; i < len; i++)
		d[i] = ((unsigned char *)&c)[i & 3];

	return dst;
}

void* memmove(void *dst, const void *src, size_t n)
{
	size_t i;
	unsigned char *d = (unsigned char *)dst;
	unsigned char *s = (unsigned char *)src;

	if(d < s) {
		for (i = 0; i < n; i++)
			d[i] = s[i];
	} else {
		for (i = n; i > 0; i--)
			d[i - 1] = s[i - 1];
	}

	return dst;
}

int memcmp(const void *p1, const void *p2, size_t n)
{
	unsigned char *a = (unsigned char *)p1;
	unsigned char *b = (unsigned char *)p2;

	for(size_t i = 0; i < n; i++) {
		if(a[i] < b[i]) 
			return -1;
		if(a[i] > b[i])
			return 1;
	}

	return 0;
}

int strcmp(const char *str1, const char *str2)
{
	const unsigned char *s1 = (const unsigned char *)str1;
	const unsigned char *s2 = (const unsigned char *)str2;

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

uint32_t simple_strtoull(const char *cp, char **endp, unsigned int base)
{
	uint32_t result = 0;

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

int simple_strtol(const char *cp, char **endp, unsigned int base)
{
	if (*cp == '-')
		return -simple_strtoull(cp + 1, endp, base);

	return simple_strtoull(cp, endp, base);
}