#include <elfboot/core.h>
#include <elfboot/string.h>

/*
 * The mem*_aligned functions assume that both the
 * destination and the source pointers are pointing 
 * which are to regions aligned on an atleast 2-byte
 * boundary. 
 * 
 * For unaligned pointers, the commonly byte-by-byte 
 * memcpy_unaligned function is called.
 */

static void* memcpy_unaligned(void *dst, const void *src, size_t len)
{
	size_t i;
	uint8_t *d = dst;
	const uint8_t *s = src;

	for (i = 0; i < len; i++)
		d[i] = s[i];

	return dst;
}

static void* memcpy_aligned16(void *dst, const void *src, size_t len)
{
	size_t i, j = len / 2;
	uint16_t *d = dst;
	const uint16_t *s = src;

	for (i = 0; i < j; i++)
		d[i] = s[i];

	/* Copy the rest of the unaligned bytes */
	memcpy_unaligned(&d[i], &s[i], len - (i * 2));

	return dst;
}

static void* memcpy_aligned32(void *dst, const void *src, size_t len)
{
	size_t i, j = len / 4;
	uint32_t *d = dst;
	const uint32_t *s = src;

	for (i = 0; i < j; i++)
		d[i] = s[i];

	/* Copy the rest of the unaligned bytes */
	memcpy_unaligned(&d[i], &s[i], len - (i * 4));

	return dst;
}

void* memcpy(void *dst, const void *src, size_t len)
{
	if (ALIGNEDMEM(dst, src, sizeof(uint32_t)))
		return memcpy_aligned32(dst, src, len);

	if (ALIGNEDMEM(dst, src, sizeof(uint16_t)))
		return memcpy_aligned16(dst, src, len);

	return memcpy_unaligned(dst, src, len);
}

void* memset(void *dst, int c, size_t len)
{
	size_t i;
	uint8_t *d = dst;

	for (i = 0; i < len; i++)
		d[i] = (uint8_t)c;

	return dst;
}

void* memset16(void *dst, int c, size_t len)
{	
	size_t i, j = len / 2;
	uint8_t *db = dst;
	uint16_t *d = dst;

	for (i = 0; i < j; i++)
		d[i] = (uint16_t)c;

	for (i *= 2; i < len; i++)
		db[i] = ((uint8_t *)&c)[i & 1];

	return dst;
}

void* memset32(void *dst, int c, size_t len)
{	
	size_t i, j = len / 4;
	uint8_t *db = dst;
	uint32_t *d = dst;

	for (i = 0; i < j; i++)
		d[i] = (uint32_t)c;

	for (i *= 4; i < len; i++)
		db[i] = ((uint8_t *)&c)[i & 3];

	return dst;
}

/*
 * memmove:
 *
 * We distinguish between several cases:
 * 
 * - The regions to be copied to not overlap. In this
 *   case, we simply call memcpy and let it handle the
 *   copy process.
 *   
 * - The regions do overlap with | dst - src | == 1.
 *   In this situation, we have to copy the bytes indi-
 *   vidually in order to prevent acidental overriding.
 *   
 * - The regions do overlap with | dst - src | > 1.
 *   For this case, we try to copy multiple bytes to
 *   the destination. We support 2- and 4-byte memmove.
 *   Note: 4-byte memmove is only possible if the dif-
 *   ference | dst - src | >= 4.
 */

static void *memmove16(void *dst, const void *src, size_t len)
{
	size_t i, j = len / 2;
	uint8_t *db = dst;
	uint16_t *d = dst;
	const uint8_t *sb = src;
	const uint16_t *s = src;

	for (i = 0; i < j; i++)
		d[i] = s[i];

	for (i *= 2; i < len; i++)
		db[i] = sb[i];

	return dst;
}

static void *memmove16_reverse(void *dst, const void *src, size_t len)
{
	size_t i, j = BOUNDARY(len, sizeof(uint16_t));
	uint8_t *db = dst;
	uint16_t *d = dst;
	const uint8_t *sb = src;
	const uint16_t *s = src;

	for (i = len; i > (len - j); i--)
		db[i - 1] = sb[i - 1];

	for (i /= 2; i > 0; i--)
		d[i - 1] = s[i - 1];

	return dst;
}

static void *memmove32(void *dst, const void *src, size_t len)
{
	size_t i, j = len / 4;
	uint8_t *db = dst;
	uint32_t *d = dst;
	const uint8_t *sb = src;
	const uint32_t *s = src;

	for (i = 0; i < j; i++)
		d[i] = s[i];

	for (i *= 4; i < len; i++)
		db[i] = sb[i];

	return dst;
}

static void *memmove32_reverse(void *dst, const void *src, size_t len)
{
	size_t i, j = BOUNDARY(len, sizeof(uint32_t));
	uint8_t *db = dst;
	uint32_t *d = dst;
	const uint8_t *sb = src;
	const uint32_t *s = src;

	for (i = len; i > (len - j); i--)
		db[i - 1] = sb[i - 1];

	for (i /= 4; i > 0; i--)
		d[i - 1] = s[i - 1];

	return dst;
}

void* memmove(void *dst, const void *src, size_t len)
{
	size_t i, diff, size;
	uint8_t *d = dst;
	const uint8_t *s = src;

	if (dst == src)
		return dst;

	if (dst < src) {
		diff = src - dst;

		if (dst + len <= src)
			return memcpy(dst, src, len);

		size = sizeof(uint32_t);
		if (ALIGNEDMEM(dst, src, size) && diff >= size)
			return memmove32(dst, src, len);

		size = sizeof(uint16_t);
		if (ALIGNEDMEM(dst, src, size) && diff >= size)
			return memmove16(dst, src, len);

		for (i = 0; i < len; i++)
			d[i] = s[i];
	} else {
		diff = dst - src;

		if (src + len <= dst)
			return memcpy(dst, src, len);

		size = sizeof(uint32_t);
		if (ALIGNEDMEM(dst, src, size) && diff >= size)
			return memmove32_reverse(dst, src, len);

		size = sizeof(uint16_t);
		if (ALIGNEDMEM(dst, src, size) && diff >= size)
			return memmove16_reverse(dst, src, len);

		for (i = len; i > 0; i--)
			d[i - 1] = s[i - 1];
	}

	return dst;
}

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