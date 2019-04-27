#ifndef __BOOT_EDD_H__
#define __BOOT_EDD_H__

int edd_read_sector(uint8_t devno, uint16_t offset, uint32_t sector);

#endif /* __BOOT_EDD_H__ */