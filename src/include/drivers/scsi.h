#ifndef __BOOT_SCSI_H__
#define __BOOT_SCSI_H__

#include <elfboot/core.h>
#include <elfboot/device.h>

#define SCSI_CMD_TEST_UNIT_READY	0x00
#define SCSI_CMD_REQUEST_SENSE		0x03
#define SCSI_CMD_INQUIRY		0x12
#define SCSI_CMD_READ_CAPACITY10	0x25
#define SCSI_CMD_READ10			0x28
#define SCSI_CMD_WRITE10		0x28
#define SCSI_CMD_READ16			0x88
#define SCSI_CMD_WRITE16		0x8A
#define SCSI_CMD_READ_CAPACITY16	0x9E
#define SCSI_CMD_READ12			0xA8
#define SCSI_CMD_WRITE12		0xAA

#define SCSI_LUN_SHIFT			5
#define SCSI_RMB_SHIFT			7

struct scsi_inquiry {
	uint8_t  cmd;
	uint8_t  lun;
	uint8_t  page;
	uint8_t  _reserved;
	uint8_t  len;
	uint8_t  control;
	uint8_t  pad[6];
} __packed;

struct scsi_inquiry_data {
	uint8_t  type;

#define SCSI_DEVICE_TYPE_MASK		(_BITUL(5) - 1)
#define SCSI_DEVICE_TYPE_DIRECT		0x00
#define SCSI_DEVICE_TYPE_CDROM		0x05

	uint8_t  rmb;
	uint16_t _reserved1;
	uint8_t  len;
	uint8_t  _reserved2[3];
	char vendor[8];
	char prodid[16];
	char prodrev[4];
} __packed;

struct scsi_request_sense {
	uint8_t  cmd;
	uint8_t  lun;
	uint8_t  _reserved1;
	uint8_t  _reserved2;
	uint8_t  len;
	uint8_t  control;
	uint8_t  pad[6];
} __packed;

struct scsi_request_sense_data {
	uint8_t  error_code;
	uint8_t  segment_number;
	uint8_t  sense_key;
	uint32_t information;
	uint8_t  additional_sense_length;
	uint32_t cmd_specific_info;
	uint8_t  additional_sense_code;
	uint8_t  additional_sense_code_qualifier;
	uint8_t  field_replaceable_unit_code;
	uint8_t  sense_key_specific[3];
} __packed;

/*
 * Read capacity structures
 */

struct scsi_read_capacity10 {
	uint8_t  cmd;
	uint8_t  lun;
	uint32_t lba;
	uint8_t  _reserved1;
	uint8_t  _reserved2;
	uint8_t  pmi;
	uint8_t  control;
	uint16_t pad;
} __packed;

struct scsi_read_capacity10_data {
	uint32_t last_block;
	uint32_t block_size;
} __packed;

struct scsi_read_capacity16 {
	uint8_t  cmd;
	uint8_t  lun;
	uint64_t lba;
	uint32_t len;
	uint8_t  pmi;
	uint8_t  control;
} __packed;

struct scsi_read_capacity16_data {
	uint64_t last_block;
	uint32_t block_size;
	uint8_t  pad[20];
} __packed;

/*
 * Valid structures for both reading and writing
 */

struct scsi_xfer10 {
	uint8_t  cmd;
	uint8_t  lun;
	uint32_t lba;
	uint8_t  _reserved1;
	uint16_t size;
	uint8_t  _reserved2;
	uint16_t pad;
} __packed;

struct scsi_xfer12 {
	uint8_t  cmd;
	uint8_t  lun;
	uint32_t lba;
	uint32_t size;
	uint8_t  _reserved;
	uint8_t  control;
} __packed;

struct scsi_xfer16 {
	uint8_t  cmd;
	uint8_t  lun;
	uint64_t lba;
	uint32_t size;
	uint8_t  _reserved;
	uint8_t  control;
} __packed;

struct scsi_driver {
	int type;
	int (*probe)(struct device *);
	int (*open)(struct device *, const char *);
	int (*read)(struct device *, char *, size_t, char *, size_t);
	int (*write)(struct device *, char *, size_t, const char *, size_t);
	int (*close)(struct device *);
	struct list_head list;

	/*
	 * The following fields are filled
	 * with driver specific information
	 */
	
	void *driver_data;
};

struct scsi_data {
	uint8_t  scsi_type;
	uint8_t  removable;
	uint64_t last_block;
	uint64_t block_size;
};

void scsi_driver_register(struct scsi_driver *driver);

void scsi_driver_unregister(struct scsi_driver *driver);

void scsi_firmware_init(void);

#endif /* __BOOT_SCSI_H__ */