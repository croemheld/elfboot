#include <elfboot/core.h>

#include <asm/bios.h>

void early_printf(const char *buffer, int length)
{
	struct biosregs ireg, oreg;
	int i = 0;

	initregs(&ireg);
	ireg.ah = 0x0e;

	while (i < length) {
		if (buffer[i] == '\n') {
			ireg.al = '\r';

			bioscall(0x10, &ireg, &oreg);
		}

		ireg.al = buffer[i++];

		bioscall(0x10, &ireg, &oreg);
	}
}