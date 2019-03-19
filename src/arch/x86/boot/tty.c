#include <boot/boot.h>

static void biosputc(int ch)
{
	struct biosregs ireg;

	initregs(&ireg);
	ireg.bx = 0x0007;
	ireg.cx = 0x0001;
	ireg.ah = 0x0e;
	ireg.al = ch;

	bioscall(0x10, &ireg, NULL);
}

void putc(int ch)
{
	if(ch == '\n')
		putc('\r');

	biosputc(ch);
}

void puts(const char *str)
{
	while(*str)
		putc(*str++);
}