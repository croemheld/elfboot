#ifndef __X86_EDD_H__
#define __X86_EDD_H__

#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/string.h>

#include <uapi/elfboot/const.h>

#define EDD_MAGIC1			0x55AA
#define EDD_MAGIC2			0xAA55

struct edd_disk_drive_params {
	uint16_t io_base;
	uint16_t control;
	uint8_t  flags;
	uint8_t  proprietary_info;
	uint8_t  irq_flags;
	uint8_t  block_count;
	uint8_t  dma_flags;
	uint8_t  pio_flags;
	uint16_t drive_options;
	uint16_t _reserved;
	uint8_t  revision;
	uint8_t  checksum;
} __packed;

#define EDD_DISK_DRIVE_PARAM_SLAVE_BIT	4
#define EDD_DISK_DRIVE_PARAM_SLAVE	_BITUL(EDD_DISK_DRIVE_PARAM_SLAVE_BIT)
#define EDD_DISK_DRIVE_PARAM_LBA_BIT	6
#define EDD_DISK_DRIVE_PARAM_LBA	_BITUL(EDD_DISK_DRIVE_PARAM_LBA_BIT)

/*
 * Drive parameters
 */

struct edd_device_params {
	uint16_t length;
	uint16_t info_flags;

#define EDD_DEVICE_PARAM_DMA_BIT	0
#define EDD_DEVICE_PARAM_DMA		_BITUL(EDD_DEVICE_PARAM_DMA_BIT)
#define EDD_DEVICE_PARAM_CHS_VALID_BIT	1
#define EDD_DEVICE_PARAM_CHS_VALID	_BITUL(EDD_DEVICE_PARAM_CHS_VALID_BIT)
#define EDD_DEVICE_PARAM_REMOVABLE_BIT	2
#define EDD_DEVICE_PARAM_REMOVABLE	_BITUL(EDD_DEVICE_PARAM_REMOVABLE_BIT)
#define EDD_DEVICE_PARAM_WRVERIFY_BIT	3
#define EDD_DEVICE_PARAM_WRVERIFY	_BITUL(EDD_DEVICE_PARAM_WRVERIFY_BIT)
#define EDD_DEVICE_PARAM_CHLINE_BIT	4
#define EDD_DEVICE_PARAM_CHLINE		_BITUL(EDD_DEVICE_PARAM_CHLINE_BIT)
#define EDD_DEVICE_PARAM_LOCKED_BIT	5
#define EDD_DEVICE_PARAM_LOCKED		_BITUL(EDD_DEVICE_PARAM_LOCKED_BIT)
#define EDD_DEVICE_PARAM_CHS_MAX_BIT	6
#define EDD_DEVICE_PARAM_CHS_MAX	_BITUL(EDD_DEVICE_PARAM_CHS_MAX_BIT)

	uint32_t num_cylinders;
	uint32_t num_heads;
	uint32_t num_sectors;
	uint64_t total_sectors;
	uint16_t bytes_per_sector;

	/* Version 2.0 */

	uint32_t dpte_ptr;

	/* Version 3.0 */

	uint16_t key;
	uint8_t  device_path_info_length;
	const uint8_t _reserved[3];
	const char host_bus_type[4];

#define EDD_DEVICE_HOST_BUS_ISA		"ISA"
#define EDD_DEVICE_HOST_BUS_PCI		"PCI"

	const char interface_type[8];

#define EDD_DEVICE_INTERFACE_ATA	"ATA"
#define EDD_DEVICE_INTERFACE_ATAPI	"ATAPI"
#define EDD_DEVICE_INTERFACE_SCSI	"SCSI"
#define EDD_DEVICE_INTERFACE_USB	"USB"
#define EDD_DEVICE_INTERFACE_1394	"1394"
#define EDD_DEVICE_INTERFACE_FIBRE	"FIBRE"

	union {
		struct {
			uint16_t base_address;
			uint16_t reserved1;
			uint32_t reserved2;
		} __packed isa;
		struct {
			uint8_t bus;
			uint8_t slot;
			uint8_t function;
			uint8_t channel;
			uint32_t reserved;
		} __packed pci;
		struct {
			uint64_t _reserved;
		} __packed ibnd;
		struct {
			uint64_t _reserved;
		} __packed xprs;
		struct {
			uint64_t _reserved;
		} __packed htpt;
		struct {
			uint64_t _reserved;
		} __packed unknown;
	} interface_path;
	union {
		struct {
			uint8_t device;
			uint8_t _reserved1;
			uint16_t _reserved2;
			uint32_t _reserved3;
			uint64_t _reserved4;
		} __packed ata;
		struct {
			uint8_t device;
			uint8_t lun;
			uint8_t _reserved1;
			uint8_t _reserved2;
			uint32_t _reserved3;
			uint64_t _reserved4;
		} __packed atapi;
		struct {
			uint16_t id;
			uint64_t lun;
			uint16_t _reserved1;
			uint32_t _reserved2;
		} __packed scsi;
		struct {
			uint64_t serial_number;
			uint64_t _reserved;
		} __packed usb;
		struct {
			uint64_t eui;
			uint64_t _reserved;
		} __packed i1394;
		struct {
			uint64_t wwid;
			uint64_t lun;
		} __packed fibre;
		struct {
			uint64_t identity_tag;
			uint64_t _reserved;
		} __packed i2o;
		struct {
			uint32_t array_number;
			uint32_t _reserved1;
			uint64_t _reserved2;
		} __packed raid;
		struct {
			uint8_t device;
			uint8_t _reserved1;
			uint16_t _reserved2;
			uint32_t _reserved3;
			uint64_t _reserved4;
		} __packed sata;
		struct {
			uint64_t _reserved1;
			uint64_t _reserved2;
		} __packed unknown;
	} device_path;
	uint8_t _reserved4;
	uint8_t checksum;
} __packed;

/*
 * Device information
 */

struct edd_device_info {
	uint8_t device;
	uint8_t version;
	uint16_t interface_support;
	uint16_t legacy_max_cylinder;
	uint8_t legacy_max_head;
	uint8_t legacy_sectors_per_track;
	struct edd_device_params params;
} __packed;

static inline int edd_device_is_type(const char *name, const char *type)
{
	return strncmp(name, type, strlen(type));
}

struct device *edd_device_create(uint8_t devno);

#endif /* __X86_EDD_H__ */