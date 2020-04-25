#include <elfboot/core.h>
#include <elfboot/mm.h>

#include <crypto/crc32.h>

/*
 * CRC32 lookup table. The table is initialized during runtime which reduces
 * the size of the elfboot bootloader more than providing a static table.
 */
static unsigned int *crc32_table = NULL;

static int crc32_init_table(void)
{
	unsigned int i, j, c;

	crc32_table = bmalloc(sizeof(unsigned int) * 256);
	if (!crc32_table)
		return -ENOMEM;

	for (i = 0; i < 256; i++) {
		for (c = i << 24, j = 8; j > 0; j--) {

			/*
			 * Perform a value calculation for the specific entry in the table.
			 * We use a custom seed for the value, namely 0x0e1fb007.
			 */
			c = c & _BITUL(31) ? (c << 1) ^ CRC32_SEED : (c << 1);
		}

		crc32_table[i] = c;
	}

	return 0;
}

unsigned int crc32(const char *buffer, unsigned int length)
{
	unsigned int crc = 0xffffffff;

	if (!crc32_table) {

		/*
		 * If this fails, then all hope is lost. Probably.
		 */
		if (crc32_init_table())
			return -EFAULT;
	}

	while (length--)
		crc = (crc << 8) ^ crc32_table[((crc >> 24) ^ *buffer++) & 255];

	return crc;
}