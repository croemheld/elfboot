#include <elfboot/core.h>
#include <elfboot/mm.h>
#include <elfboot/io.h>
#include <elfboot/device.h>
#include <elfboot/string.h>
#include <elfboot/printf.h>
#include <elfboot/list.h>

#include <drivers/scsi.h>
#include <drivers/ata.h>

/*
 * We are currently in the early-boot stage of our OS. At this point, it 
 * wouldn't make sense to support writing to devices, since we only want 
 * to load files from the devices into memory.
 */

static void initcommand(struct ata_command *command)
{
	memset(command, 0, sizeof(*command));
}

static inline void ata_set_reg(struct device *device, int reg, int val)
{
	outb(device->io->io_base + reg, val);
}

static inline uint8_t ata_get_reg(struct device *device, int reg)
{
	return inb(device->io->io_base + reg);
}

static void ata_pio_read(struct device *device, uint16_t *buf, uint64_t size)
{
	uint64_t i;

	for (i = 0; i < (size / 2); i++)
		*(buf + i) = inw(device->io->io_base + ATA_REG_DATA);
}

static void ata_pio_write(struct device *device, uint16_t *buf, uint64_t size)
{
	uint64_t i;

	for (i = 0; i < (size / 2); i++)
		outw(device->io->io_base + ATA_REG_DATA, *(buf + i));
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

	port = device->io->io_base + ATA_REG_STATUS;

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

static int ata_handle(struct device *device, struct ata_command *atacmd)
{
	int status, slave, hdsel, i;
	uint8_t irs;
	uint32_t cnt, nread = 0;

	/* Determine the selected device */
	slave = device_has(device, DEVICE_FLAG_SLAVE);
	hdsel = (atacmd->regs.disk & 0xef) | (slave << 4);

	ata_set_reg(device, ATA_REG_HDDEVSEL, hdsel);

	/* Send parameters */
	for (i = ATA_REG_SECTORS; i <= ATA_REG_LBA2; i++)
		ata_set_reg(device, i, 
			    atacmd->regs.raw[7 + (i - ATA_REG_SECTORS)]);

	for (i = ATA_REG_FEATURES; i <= ATA_REG_LBA2; i++)
		ata_set_reg(device, i, 
			    atacmd->regs.raw[i - ATA_REG_FEATURES]);

	ata_set_reg(device, ATA_REG_COMMAND, atacmd->regs.cmd);

	/* Pull the status register */
	if (ata_wait(device, 0))
		return -1;

	status = ata_get_reg(device, ATA_REG_STATUS);

	if (atacmd->cmdsize) {
		if (ata_wait(device, 0))
			return -1;

		irs = ata_get_reg(device, ATAPI_REG_IREASON);

		if (!((status & ATA_SR_DRQ) 
		    && (irs & ATAPI_IREASON_MASK) == ATAPI_IREASON_CMD_OUT))
			return -1;

		ata_pio_write(device, atacmd->cmd, atacmd->cmdsize);
	}

	/* Transfer data */
	while (nread < atacmd->bufsize 
	       && (status & (ATA_SR_DRQ | ATA_SR_ERR)) == ATA_SR_DRQ) {
		if (ata_wait(device, 0))
			return -1;

		if (atacmd->cmdsize) {
			irs = ata_get_reg(device, ATAPI_REG_IREASON);

			if ((irs & ATAPI_IREASON_MASK) != ATAPI_IREASON_DATA_IN)
				return -1;

			cnt = ata_get_reg(device, ATAPI_REG_CNTHIGH) << 8
				| ata_get_reg(device, ATAPI_REG_CNTLOW);

			if (!(0 < cnt && cnt <= atacmd->bufsize - nread
			    && (!(cnt & 1) || cnt == atacmd->bufsize - nread)))
				return -1;
		} else
			cnt = 512;

		if (cnt > atacmd->bufsize - nread)
			cnt = atacmd->bufsize - nread;

		if (atacmd->write)
			ata_pio_write(device, atacmd->buf + nread, cnt);
		else 
			ata_pio_read(device, atacmd->buf + nread, cnt);

		nread += cnt;
	}

	if (atacmd->write) {
		if(ata_wait(device, 0))
			return -1;

		status = ata_get_reg(device, ATA_REG_STATUS);

		if (status & (ATA_SR_DRQ | ATA_SR_ERR))
			return -1;
	}

	atacmd->bufsize = nread;

	if (ata_wait(device, 0))
		return -1;

	for (i = ATA_REG_ERROR; i <= ATA_REG_STATUS; i++)
		atacmd->regs.raw[i - ATA_REG_FEATURES] = ata_get_reg(device, i);

	if (atacmd->regs.status & (ATA_SR_DRQ | ATA_SR_ERR))
		return -1;

	return 0;
}

static int ata_identify(struct device *device)
{
	int r;
	struct ata_command atacmd;

	initcommand(&atacmd);
	atacmd.buf = bmalloc(ATA_IDENTIFY_SIZE);

	if (!atacmd.buf)
		return -ENOMEM;

	atacmd.bufsize = ATA_IDENTIFY_SIZE;

	atacmd.regs.disk = 0xE0;
	atacmd.regs.cmd = ATA_CMD_IDENTIFY;

	r = ata_handle(device, &atacmd);

	if (r || atacmd.bufsize != ATA_IDENTIFY_SIZE)
		r = -EFAULT;

	bfree(atacmd.buf);

	return 0;
}

static int atapi_identify(struct device *device)
{
	int r;
	struct ata_command atacmd;

	initcommand(&atacmd);
	atacmd.buf = bmalloc(ATA_IDENTIFY_SIZE);

	if (!atacmd.buf)
		return -ENOMEM;

	atacmd.bufsize = ATA_IDENTIFY_SIZE;

	atacmd.regs.disk = 0xE0;
	atacmd.regs.cmd = ATA_CMD_IDENTIFY_PACKET;

	r = ata_handle(device, &atacmd);

	if (r) {
		bfree(atacmd.buf);

		return -EFAULT;
	}

	bfree(atacmd.buf);

	return 0;
}

static int ata_probe(struct device *device)
{
	return ata_identify(device);
}

static int ata_open(struct device *device __unused, const char *name __unused)
{
	return -ENOTSUP;
}

static int ata_read(struct device *device __unused, uint64_t sector __unused, 
		    uint64_t size __unused, char *buffer __unused)
{
	return -ENOTSUP;
}

static int ata_write(struct device *device __unused, uint64_t sector __unused, 
		     uint64_t size __unused, const char *buffer __unused)
{
	return -ENOTSUP;
}

static int ata_close(struct device *device __unused)
{
	return -ENOTSUP;
}

static int atapi_probe(struct device *device)
{
	return atapi_identify(device);
}

static int atapi_open(struct device *device __unused, const char *name __unused)
{
	return -ENOTSUP;
}

static int atapi_read(struct device *device, char *cmd, size_t cmdsize, 
		      char *buffer, size_t size)
{
	struct ata_command atacmd;
	int r;

	initcommand(&atacmd);
	atacmd.regs.disk = 0;
	atacmd.regs.features = 0;
	atacmd.regs.atapi_ireason = 0;
	atacmd.regs.atapi_cnthigh = size >> 8;
	atacmd.regs.atapi_cntlow = size & 0xff;
	atacmd.regs.cmd = ATA_CMD_PACKET;

	atacmd.cmd = cmd;
	atacmd.cmdsize = cmdsize;
	atacmd.buf = buffer;
	atacmd.bufsize = size;

	r = ata_handle(device, &atacmd);

	if (r || atacmd.bufsize != size)
		return r;

	return 0;
}

static int atapi_write(struct device *device __unused, char *cmd __unused, 
		       size_t cmdsize __unused, const char *buffer __unused,
		       size_t size __unused)
{
	bprintln("ATAPI write not supported!\n");

	return -ENOTSUP;
}

static int atapi_close(struct device *device __unused)
{
	return -ENOTSUP;
}

static struct device_driver ata_device_driver = {
	.probe = ata_probe,
	.open = ata_open,
	.read = ata_read,
	.write = ata_write,
	.close = ata_close,
	.list = LIST_HEAD_INIT(ata_device_driver.list),
};

static struct scsi_driver atapi_device_driver = {
	.probe = atapi_probe,
	.open = atapi_open,
	.read = atapi_read,
	.write = atapi_write,
	.close = atapi_close,
	.list = LIST_HEAD_INIT(atapi_device_driver.list),
};

void ata_firmware_init(void)
{
	device_driver_register(&ata_device_driver);
	scsi_driver_register(&atapi_device_driver);
}