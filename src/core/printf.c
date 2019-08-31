#include <elfboot/core.h>
#include <elfboot/string.h>
#include <elfboot/console.h>

/*
 * Private structures
 */

typedef struct {
    char *buffer;
    char *end;
    uint32_t flags;
    int width;
    bool precision;
    int strlen;
} formatter;

typedef enum {
    PADDING_NUL                                   = 0x00000001,
    PADDING_NEG                                   = 0x00000002,
} format_flags_t;

/*
 * Functions
 */

void put_char(formatter *f, char c)
{
	if (f->buffer < f->end)
		*f->buffer++ = c;
}

void put_string(formatter *f, const char *str)
{
	int width = f->width;
	char pad_char = (f->flags & PADDING_NUL) ? '0' : ' ';

	if (~f->flags & PADDING_NEG) {
		while(--width >= 0)
			put_char(f, pad_char);
	}

	if (!f->precision)
		while(*str)
			put_char(f, *str++);
	else {
		f->precision = false;

		while(--f->strlen >= 0)
			put_char(f, *str++);
	}

	while(--width >= 0)
		put_char(f, pad_char);
}

void put_decimal(formatter *f, unsigned long long n)
{
	char buffer[32];
	char *end = buffer + sizeof(buffer) - 1;
	char *str = end;

	/* NULL terminator */
	*str = '\0';

	do {
		char c = '0' + (n % 10);
		*--str = c;

		n /= 10;
	} while(n > 0);

	f->width -= end - str;

	put_string(f, str);
}

void put_hexadecimal(formatter *f, char type, unsigned long long n)
{
	char buffer[32];
	char *end = buffer + sizeof(buffer) - 1;
	char *str = end;

	char c;

	/* NULL terminator */
	*str = '\0';

	do {
		uint8_t digit = n & 0xf;

		if (digit < 10)
			c = '0' + digit;
		else if (type == 'x')
			c = 'A' + digit - 10;
		else
			c = 'A' + digit - 10;

		*--str = c;
		n >>= 4;
	} while(n > 0);

	f->width -= end - str;

	put_string(f, str);
}

void put_pointer(formatter *f, void *ptr)
{
	uint32_t n = (uintptr_t)ptr;

	put_hexadecimal(f, 'x', n);
}

int vsnprintf(char *buffer, size_t size, const char *format, va_list *argp)
{
	formatter f;

	f.buffer = buffer;
	f.end = buffer + size - 1;

	while(1) {
		char c = *format++;
		bool is_long_long = false;

		if (!c)
			break;

		if (c != '%') {
			put_char(&f, c);
			continue;
		}

		c = *format++;

		f.flags = 0;

		if (c == '-') {
			f.flags |= PADDING_NEG;
			c = *format++;
		} else if (c == '0') {
			f.flags |= PADDING_NUL;
			c = *format++;
		}

		f.width  = -1;
		f.strlen = -1;
		f.precision = false;

		if (c == '.') {
			f.precision = true;
			c = *format++;

			if (c == '*') {
				f.strlen = va_arg(*argp, int);
				c = *format++;
			}
		}

		if (is_digit(c)) {
			int width = 0;

			do {
				width = width * 10 + c - '0';
				c = *format++;
			} while(is_digit(c));

			if (f.precision && !(f.strlen > 0))
				f.strlen = width;
			else
				f.width = width;
		}

		if (c == 'l') {
			c = *format++;

			if (c == 'l') {
				c = *format++;
				is_long_long = true;
			}
		}

		char type = c;

		switch(type) {
			case '%': 
			{
				put_char(&f, '%');
				break;
			}

			case 'c': 
			{
				c = va_arg(*argp, int);
				put_char(&f, c);
				break;
			}

			case 's': 
			{
				char *s = va_arg(*argp, char *);

				if (!s)
					s = "(null)";

				if (f.width > 0) {
					char *ptr = s;

					while(*ptr)
						++ptr;

					f.width -= ptr - s;
				}

				put_string(&f, s);
				break;
			}

			case 'd': 
			{
				long long n;

				if (is_long_long) {
					n = va_arg(*argp, long long);
				} else
					n = va_arg(*argp, int);

				if (n < 0) {
					put_char(&f, '-');
					n = -n;
				}

				put_decimal(&f, n);
				break;
			}

			case 'u': 
			{
				unsigned long long n;

				if (is_long_long) {
					n = va_arg(*argp, unsigned long long);
				} else
					n = va_arg(*argp, unsigned long);

				put_decimal(&f, n);
				break;
			}

			case 'x':
			case 'X': 
			{
				unsigned long long n;

				if (is_long_long) {
					n = va_arg(*argp, unsigned long long);

				} else
					n = va_arg(*argp, unsigned long);

				put_hexadecimal(&f, type, n);
				break;
			}

			case 'p': 
			{
				void *p = va_arg(*argp, void *);

				put_char(&f, '0');
				put_char(&f, 'x');
				put_pointer(&f, p);
				break;
			}
		}
	}


	if (f.buffer < f.end + 1)
		*f.buffer = '\0';

	return f.buffer - buffer;
}

int snprintf(char *buffer, size_t size, const char *format, ...)
{
	va_list argp;

	va_start(argp, format);
	int length = vsnprintf(buffer, size, format, &argp);
	va_end(argp);

	return length;
}

int sprintf(char* buffer, const char *format, ...)
{
	va_list argp;

	va_start(argp, format);
	int length = vsnprintf(buffer, strlen(buffer), format, &argp);
	va_end(argp);

	return length;
}

int bprintf(const char *format, ...)
{
	va_list argp;
	char buffer[256];

	va_start(argp, format);
	int length = vsnprintf(buffer, 256, format, &argp);
	va_end(argp);

	console_write_active(buffer, length);

	return length;
}