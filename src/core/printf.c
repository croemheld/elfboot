#include <elfboot/core.h>
#include <elfboot/string.h>
#include <elfboot/console.h>
#include <elfboot/file.h>
#include <elfboot/printf.h>
#include <elfboot/math.h>

#include <elfboot/debug.h>

/*
 * File descriptor to console
 */
static struct file *fcons = NULL;

/*
 * Individual handlers for characters, strings and numbers
 */

static void vsprintf_put_char(struct format_struct *fmts, int c)
{
	if (!(fmts->flags & PAD_NEG)) 
		while (--fmts->pad > 0)
			*fmts->buf++ = ' ';

	*fmts->buf++ = c;

	while (--fmts->pad > 0)
		*fmts->buf++ = ' ';
}

static void vsprintf_put_string(struct format_struct *fmts, const char *str)
{
	int i, len = strnlen(str, fmts->pre);

	if (!(fmts->flags & PAD_NEG))
		while (fmts->pad-- > len)
			*fmts->buf++ = ' ';

	for (i = 0; i < len; ++i)
		*fmts->buf++ = *str++;

	while (fmts->pad-- > len)
		*fmts->buf++ = ' ';
}

static __always_inline int vsprintf_base(char qual)
{
	if (qual == 'b')
		return 2;

	if (qual == 'o')
		return 8;

	if (qual == 'd' || qual == 'u')
		return 10;

	if (qual == 'x')
		return 16;

	return 0;
}

static void vsprintf_put_number(struct format_struct *fmts,
	long long n, int base)
{
	/*
	 * Lookup array of allowed digits
	 */
	static const char digits[16] = "0123456789ABCDEF";

	char c, sign, buf[66];
	int i, d;

	/*
	 * That should not happen!
	 */
	if (!base)
		return;

	/*
	 * Signedness of number
	 */
	c = (fmts->flags & PAD_NUL) ? '0' : ' ';
	sign = 0;
	if (fmts->flags & SGN_POS) {
		if (n < 0) {
			sign = '-';
			n = -n;
			fmts->pad--;
		}
	}

	i = 0;
	if (!n)
		buf[i++] = '0';
	else
		while (n != 0) {
			d = (unsigned long long)n % (unsigned int)base;
			n = (unsigned long long)n / (unsigned int)base;
			buf[i++] = digits[d];
		}

	if (i > fmts->pre)
		fmts->pre = i;

	fmts->pad -= fmts->pre;
	if (!(fmts->flags & (PAD_NEG | PAD_NUL)))
		while (fmts->pad-- > 0)
			*fmts->buf++ = ' ';

	if (sign)
		*fmts->buf++ = sign;

	if (!(fmts->flags & PAD_NEG))
		while (fmts->pad-- > 0)
			*fmts->buf++ = c;

	while (i < fmts->pre)
		*fmts->buf++ = '0';

	while (i-- > 0)
		*fmts->buf++ = buf[i];

	while (fmts->pad-- > 0)
		*fmts->buf++ = ' ';
}

static int vnsprintf_number(const char **str)
{
	int num = 0;

	while (is_digit(**str))
		num = num * 10 + *((*str)++) - '0';

	return num;
}

/*
 * Main function for transofrming strings
 */

int vsprintf(char *buf, const char *fmt, va_list *argp)
{
	struct format_struct fmts;
	unsigned long long num;
	bool is_long_long;

	for (fmts.buf = buf; *fmt; ++fmt) {
		is_long_long = false;
		
		if (*fmt != '%') {
			*fmts.buf++ = *fmt;
			continue;
		}

		fmts.flags = 0;

next_char:

		switch(*++fmt) {
			case '-':
				fmts.flags |= PAD_NEG;
				goto next_char;
			case '0':
				fmts.flags |= PAD_NUL;
				goto next_char;
		}

		/*
		 * Indentation
		 */

		fmts.pad = -1;
		if (is_digit(*fmt))
			fmts.pad = vnsprintf_number(&fmt);
		else if (*fmt == '*') {
			++fmt;
			fmts.pad = va_arg(*argp, int);
			if (fmts.pad < 0) {
				fmts.pad = -fmts.pad;
				fmts.flags |= PAD_NEG;
			}
		}

		/*
		 * Precision (for strings only)
		 */

		fmts.pre = -1;
		if (*fmt == '.') {
			++fmt;
			if (is_digit(*fmt))
				fmts.pre = vnsprintf_number(&fmt);
			else if (*fmt == '*') {
				++fmt;
				fmts.pre = va_arg(*argp, int);
			}

			if (fmts.pre < 0)
				fmts.pre = 0;
		}

		/*
		 * Large numbers (up to 64-bit)
		 */

		if (*fmt == 'l') {
			++fmt;
			if (*fmt == 'l') {
				++fmt;
				is_long_long = true;
			}
		}

		/*
		 * Argument qualifiers
		 */

		switch (*fmt) {
			case 'c':
				vsprintf_put_char(&fmts, va_arg(*argp, int));
				continue;
			case 's':
				vsprintf_put_string(&fmts, va_arg(*argp, char *));
				continue;
			case 'b':
			case 'o':
			case 'd':
			case 'u':
			case 'x':
				if (*fmt == 'd')
					fmts.flags |= SGN_POS;

				if (is_long_long)
					num = va_arg(*argp, unsigned long long);
				else
					num = va_arg(*argp, unsigned long);

				vsprintf_put_number(&fmts, num, vsprintf_base(*fmt));
				continue;
			default:
				*fmts.buf++ = '%';
				if (*fmt)
					*fmts.buf++ = *fmt;
				else
					--fmt;
				continue;
		}
	}

	*fmts.buf = '\0';

	return fmts.buf - buf;
}

int sprintf(char *buffer, const char *format, ...)
{
	va_list argp;
	int length;

	va_start(argp, format);
	length = vsprintf(buffer, format, &argp);
	va_end(argp);

	return length;
}

int bprintf(const char *format, ...)
{
	va_list argp;
	char buffer[256];
	int length;

	va_start(argp, format);
	length = vsprintf(buffer, format, &argp);
	va_end(argp);

	file_write(fcons, length, buffer);

#ifdef CONFIG_DEBUG
	debug_printf(buffer, length);
#endif

	return length;
}

int console_init(const char *path)
{
	fcons = file_open(path, 0);
	if (!fcons)
		return -EFAULT;

	return 0;
}