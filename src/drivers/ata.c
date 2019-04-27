#include <asm/boot.h>

#include "device.h"
#include "disk.h"
#include "alloc.h"
#include "edd.h"
#include "scsi.h"
#include "ata.h"

/*
 * We are currently in the early-boot stage of our OS. At this point, it 
 * wouldn't make sense to support writing to devices, since we only want 
 * to load files from the devices into memory.
 */

static void initparams(struct ata_params *params)
{
	memset(params, 0, sizeof(*params));
}

static inline void ata_set_reg(struct device *device, int reg, int val)
{
	outb(device->params->io_base + reg, val);
}

static inline uint8_t ata_get_reg(struct device *device, int reg)
{
	return inb(device->params->io_base + reg);
}

static void ata_pio_read(struct device *device, uint16_t *buf, uint64_t size)
{
	uint64_t i;

	for (i = 0; i < (size / 2); i++)
		*(buf + i) = inw(device->params->io_base + ATA_REG_DATA);
}

static void ata_pio_write(struct device *device, uint16_t *buf, uint64_t size)
{
	uint64_t i;

	for (i = 0; i < (size / 2); i++)
		outw(device->params->io_base + ATA_REG_DATA, *(buf + i));
}

static void ata_io_wait(struct device *device)
{
	/*
	 * See https://wiki.osdev.org/ATA_PIO_Mode#400ns_delays
	 */

	ata_get_reg(device, ATA_REG_ALTSTATUS);
	ata_get_reg(device, ATA_REG_ALTSTATUS);
	ata_get_reg(device, ATA_REG_ALTSTATUS);
	ata_get_reg(device, ATA_REG_ALTSTATUS);
}

static int ata_status_wait(struct device *device, int timeout)
{
	uint16_t port;
	int status, i;

	port = device->params->io_base + ATA_REG_STATUS;

	if (timeout > 0) {
		i = 0;

		while((status = inb(port)) & ATA_SR_BSY && (i < timeout))
			i++;
	} else
		while((status = inb(port)) & ATA_SR_BSY);

	return status;
}

static int ata_wait(struct device *device, int advanced)
{
	uint8_t status;

	/* 400ns delay */
	ata_io_wait(device);

	/* Wait until !ATA_SR_BSY */
	status = ata_status_wait(device, ATA_IO_TIMEOUT);

	if (advanced) {
		if (status & ATA_SR_ERR)
			return 1;

		if (status & ATA_SR_DF)
			return 1;

		if (!(status & ATA_SR_DRQ))
			return 1;
	}

	return 0;
}

static int ata_handle(struct device *device, struct ata_params *params)
{
	int status, slave, hdsel, i;
	uint8_t irs;
	uint32_t cnt, nread = 0;

	/* Determine the selected device */
	slave = device_is_slave(device);
	hdsel = (params->regs.disk & 0xef) | (slave << 4);

	ata_set_reg(device, ATA_REG_HDDEVSEL, hdsel);

	/* Send parameters */
	for (i = ATA_REG_SECTORS; i <= ATA_REG_LBA2; i++)
		ata_set_reg(device, i, 
			    params->regs.raw[7 + (i - ATA_REG_SECTORS)]);

	for (i = ATA_REG_FEATURES; i <= ATA_REG_LBA2; i++)
		ata_set_reg(device, i, 
			    params->regs.raw[i - ATA_REG_FEATURES]);

	ata_set_reg(device, ATA_REG_COMMAND, params->regs.cmd);

	/* Pull the status register */
	if (ata_wait(device, 1))
		return -1;

	status = ata_get_reg(device, ATA_REG_STATUS);

	if (params->cmdsize) {
		if (ata_wait(device, 1))
			return -1;

		irs = ata_get_reg(device, ATAPI_REG_IREASON);

		if (!((status & ATA_SR_DRQ) 
		    && (irs & ATAPI_IREASON_MASK) == ATAPI_IREASON_CMD_OUT))
			return -1;

		ata_pio_write(device, params->cmd, params->cmdsize);
	}

	/* Transfer data */
	while (nread < params->bufsize 
	       && (status & (ATA_SR_DRQ | ATA_SR_ERR)) == ATA_SR_DRQ) {
		if (ata_wait(device, 1))
			return -1;

		if (params->cmdsize) {
			irs = ata_get_reg(device, ATAPI_REG_IREASON);

			if ((irs & ATAPI_IREASON_MASK) != ATAPI_IREASON_DATA_IN)
				return -1;

			cnt = ata_get_reg(device, ATAPI_REG_CNTHIGH) << 8
				| ata_get_reg(device, ATAPI_REG_CNTLOW);

			if (!(0 < cnt && cnt <= params->bufsize - nread
			    && (!(cnt & 1) || cnt == params->bufsize - nread)))
				return -1;
		} else
			cnt = 512;

		if (cnt > params->bufsize - nread)
			cnt = params->bufsize - nread;

		if (params->write)
			ata_pio_write(device, params->buf + nread, cnt);
		else 
			ata_pio_read(device, params->buf + nread, cnt);

		nread += cnt;
	}

	if (params->write) {
		if(ata_wait(device, 1))
			return -1;

		status = ata_get_reg(device, ATA_REG_STATUS);

		if (status & (ATA_SR_DRQ | ATA_SR_ERR))
			return -1;
	}

	params->bufsize = nread;

	if (ata_wait(device, 1))
		return -1;

	for (i = ATA_REG_ERROR; i <= ATA_REG_STATUS; i++)
		params->regs.raw[i - ATA_REG_FEATURES] = ata_get_reg(device, i);

	if (params->regs.status & (ATA_SR_DRQ | ATA_SR_ERR))
		return -1;

	return 0;
}

static int atapi_identify(struct device *device)
{
	struct ata_params params;

	initparams(&params);
	params.buf = bmalloc(DISK_SECTOR_SIZE);

	if (!params.buf)
		return -1;

	params.bufsize = DISK_SECTOR_SIZE;

	params.regs.disk = 0xE0;
	params.regs.cmd = ATA_CMD_IDENTIFY_PACKET;

	if (ata_handle(device, &params)) {
		bfree(params.buf);

		return 1;
	}

	return 0;
}

static int ata_identify(struct device *device)
{
	struct ata_params params;

	if (device_is_atapi(device))
		return atapi_identify(device);

	initparams(&params);
	params.buf = bmalloc(DISK_SECTOR_SIZE);

	if (!params.buf)
		return -1;

	params.bufsize = DISK_SECTOR_SIZE;

	params.regs.disk = 0xE0;
	params.regs.cmd = ATA_CMD_IDENTIFY;

	if (ata_handle(device, &params) || params.bufsize != DISK_SECTOR_SIZE) {
		bfree(params.buf);

		/* Try ATAPI */
		return atapi_identify(device);
	}

	return 0;
}

static int ata_open(const char *name __unused, struct device *device)
{
	return ata_identify(device);
}

static int ata_read(struct device *device, uint64_t sector, 
		    uint64_t size, char *buffer)
{
	return -1;
}

static int ata_write(struct device *device, uint64_t sector, 
		     uint64_t size, const char *buffer)
{
	return -1;
}

static int ata_close(struct device *device)
{
	return -1;
}

static int atapi_open(const char *name __unused, struct device *device)
{
	return ata_identify(device);
}

static int atapi_read(struct device *device, uint64_t sector, 
		    uint64_t size, char *buffer)
{
	uint8_t *cmd;
	struct ata_params params;

	initparams(&params);
	params.regs.disk = 0;
	params.regs.features = 0;
	params.regs.atapi_ireason = 0;
	params.regs.atapi_cnthigh = size >> 8;
	params.regs.atapi_cntlow = size & 0xff;
	params.regs.cmd = ATA_CMD_PACKET;

	cmd = bmalloc(12);

	if (!cmd)
		return -1;

	

	return 0;
}

static int atapi_write(struct device *device, uint64_t sector, 
		     uint64_t size, const char *buffer)
{
	bprintf("ATAPI write not supported!\n");

	return -1;
}

static int atapi_close(struct device *device)
{
	return -1;
}

/* -------------------------------------------------------------------------- */

static struct disk_operations ata_disk_operations = {
	.firmware_name = "ATA",
	.open = ata_open,
	.read = ata_read,
	.write = ata_write,
	.close = ata_close
};

static struct disk_operations atapi_disk_operations = {
	.firmware_name = "ATAPI",
	.open = atapi_open,
	.read = atapi_read,
	.write = atapi_write,
	.close = atapi_close
};

void ata_firmware_init(void)
{
	disk_firmware_register(&ata_disk_operations);
	disk_firmware_register(&atapi_disk_operations);
}