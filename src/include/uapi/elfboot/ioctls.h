#ifndef __UAPI_ELFBOOT_IOCTLS_H__
#define __UAPI_ELFBOOT_IOCTLS_H__

#define IOCTL(x)		(x) | (0xEB << 8)

#define IOCTL_RESET		0x00
#define IOCTL_CLEAN		0x01

/*
 * IOCTLs for TTY
 */

#define IOCTL_PAINT		0x02
#define IOCTL_GATTR		0x03

#endif /* __UAPI_ELFBOOT_IOCTLS_H__ */