#ifndef __BOOT_SCSI_H__
#define __BOOT_SCSI_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

struct scsi_xfer10 {
	uint8_t  cmd;
	uint8_t  lun;
	uint32_t lba;
	uint8_t  _reserved;
	uint16_t size;
	uint8_t  _reserved2;
	uint16_t pad;
} __attribute__((packed));

struct scsi_xfer12 {
	uint8_t  cmd;
	uint8_t  lun;
	uint32_t lba;
	uint32_t size;
	uint8_t  _reserved;
	uint8_t  control;
} __attribute__((packed));

struct scsi_xfer16 {
	uint8_t  cmd;
	uint8_t  lun;
	uint64_t lba;
	uint32_t size;
	uint8_t  _reserved;
	uint8_t  control;
} __attribute__((packed));

#endif /* __BOOT_SCSI_H__ */