#ifndef __CRYPTO_CRC32_H__
#define __CRYPTO_CRC32_H__

#define DRIVER_CRC	"CRC"

#define CRC32_SEED	0x0e1fb007

unsigned int crc32(const char *buffer, unsigned int length);

#endif /* __CRYPTO_CRC32_H__ */