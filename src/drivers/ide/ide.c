#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/mm.h>
#include <elfboot/io.h>
#include <elfboot/module.h>
#include <elfboot/bdev.h>
#include <elfboot/pci.h>
#include <elfboot/libata.h>
#include <elfboot/string.h>
#include <elfboot/printf.h>

#include <drivers/ide.h>
#include <drivers/ata.h>
#include <drivers/scsi.h>

static uint16_t ide_ports[] = {
	IDE_PORT_PRIMARY,
	IDE_PORT_SECONDARY,
};

/*
 * Utility functions for IDE/ATA/ATAPI devices
 */

static void ide_outb(struct ide_dev *idedev, int reg, int val)
{
	outb(idedev->io_base + reg, val);
}

static uint8_t ide_inb(struct ide_dev *idedev, int reg)
{
	return inb(idedev->io_base + reg);
}

static void ide_read_io(struct ide_dev *idedev, uint16_t *buf, uint64_t size)
{
	uint64_t read;

	for (read = 0; read < (size >> 1); read++) {

		/*
		 * Read from ATA_REG_DATA, until we reached the specified number of
		 * bytes into the given buffer. To simplify it we read from the IDE
		 * device in word granularity.
		 */
		buf[read] = inw(idedev->io_base + ATA_REG_DATA);
	}
}

static void ide_write_io(struct ide_dev *idedev, uint16_t *buf, uint64_t size)
{
	uint64_t write;

	for (write = 0; write < (size >> 1); write++) {

		/*
		 * Send the parameters to ATA_REG_DATA in word chunks until we sent
		 * all values to the target port.
		 */
		outw(idedev->io_base + ATA_REG_DATA, buf[write]);
	}
}

static void ide_poll_piodel(struct ide_dev *idedev)
{
	/*
	 * See https://wiki.osdev.org/ATA_PIO_Mode#400ns_delays
	 */

	ide_inb(idedev, ATA_REG_ALTSTATUS);
	ide_inb(idedev, ATA_REG_ALTSTATUS);
	ide_inb(idedev, ATA_REG_ALTSTATUS);
	ide_inb(idedev, ATA_REG_ALTSTATUS);
}

static uint8_t ide_poll_status(struct ide_dev *idedev)
{
	uint32_t time;
	uint8_t  status;

	for (time = 0; time < ATA_IO_TIMEOUT; time++) {

		/*
		 * Wait actively until either IDE device is not busy anymore or up
		 * until the timeout value has been reached even though the device
		 * is still busy.
		 */
		status = ide_inb(idedev, ATA_REG_STATUS);
		if (!(status & ATA_SR_BSY))
			break;
	}

	return status;
}

static uint8_t ide_poll_io(struct ide_dev *idedev)
{
	ide_poll_piodel(idedev);

	return ide_poll_status(idedev);
}

/*
 * IDE device operations
 */

static int ide_send_command(struct ide_dev *idedev, struct ata_cmd *cmd)
{
	uint8_t status, dev;
	ata_regs_t *reg;
	uint32_t i, cnt, rem, nbread;

	reg = &cmd->reg;
	dev = (reg->disk & 0xef) | (idedev->slave << 4);

	/*
	 * Core function for handling read/write operation on a device connexted
	 * to the PCI IDE controller. This function works for both ATA and ATAPI.
	 */

	ide_outb(idedev, ATA_REG_HDDEVSEL, dev);

	/*
	 * Send data to selected drive by iterating over raw bytes of packet. At
	 * the end, we will send the actual command to the ATA/ATAPI device.
	 */
	for (i = ATA_REG_SECTORS; i <= ATA_REG_LBAHIGH; i++)
		ide_outb(idedev, i, reg->raw[7 + (i - ATA_REG_SECTORS)]);
	for (i = ATA_REG_FEATURES; i <= ATA_REG_LBAHIGH; i++)
		ide_outb(idedev, i, reg->raw[i - ATA_REG_FEATURES]);

	ide_outb(idedev, ATA_REG_COMMAND, reg->cmd);
	status = ide_poll_io(idedev);

	/*
	 * Send an ATAPI packet to the IDE device only if a a size and the packet
	 * itself are available. For now, only ATAPI is supported.
	 */
	if (cmd->cmdsize && cmd->cmd) {
		if ((status & ATA_SR_POLL) != ATA_SR_DRQ)
			return -EFAULT;

		status = ide_inb(idedev, ATAPI_REG_IREASON);
		if (!((status & ATAPI_IREASON_MASK) == ATAPI_IREASON_CMD_OUT))
			return -EFAULT;

		ide_write_io(idedev, cmd->cmd, cmd->cmdsize);
		status = ide_poll_io(idedev);
	}

	nbread = 0;
	while (nbread < cmd->bufsize) {
		rem  = cmd->bufsize - nbread;

		if ((status & ATA_SR_POLL) != ATA_SR_DRQ)
			break;

		if (cmd->cmdsize) {
			status = ide_inb(idedev, ATAPI_REG_IREASON);
			if ((status & ATAPI_IREASON_MASK) != ATAPI_IREASON_DATA_IN)
				return -EFAULT;

			cnt = ide_inb(idedev, ATAPI_REG_CNTHIGH) << 8
				| ide_inb(idedev, ATAPI_REG_CNTLOW);

			if (!(cnt > 0 && cnt <= rem && (!(cnt & 1) || cnt == rem)))
				return -EFAULT;
		} else
			cnt = 512;
		
		if (cnt > rem)
			cnt = rem;

		if (cmd->write)
			ide_write_io(idedev, cmd->buf + nbread, cnt);
		else
			ide_read_io(idedev, cmd->buf + nbread, cnt);

		nbread += cnt;
		status = ide_poll_io(idedev);
	}

	if (cmd->write) {
		status = ide_poll_io(idedev);
		if (status & (ATA_SR_DRQ | ATA_SR_ERR))
			return -EFAULT;
	}

	cmd->bufsize = nbread;
	ide_poll_io(idedev);

	for (i = ATA_REG_ERROR; i < ATA_REG_STATUS; i++)
		reg->raw[i - ATA_REG_FEATURES] = ide_inb(idedev, i);

	if (reg->status & (ATA_SR_DRQ | ATA_SR_ERR))
		return -EFAULT;

	return 0;
}

static int ide_atapi_send(struct bdev *bdev, void *pkg, size_t pkglen,
	void *buf, size_t buflen)
{
	struct ata_cmd cmd = { 0 };

	cmd.reg.atapi_cnthigh = (buflen >> 8) & 0xff;
	cmd.reg.atapi_cntlow  = (buflen >> 0) & 0xff;
	cmd.reg.cmd = ATA_CMD_PACKET;

	cmd.cmd = pkg;
	cmd.cmdsize = pkglen;
	cmd.buf = buf;
	cmd.bufsize = buflen;

	return (ide_send_command(bdev->private, &cmd) || cmd.bufsize != buflen);
}

static int ide_atapi_request_sense(struct bdev *bdev)
{
	struct scsi_request_sense_data rsd;
	struct scsi_request_sense rs = {
		.cmd = SCSI_CMD_REQUEST_SENSE,
		.len = sizeof(rs)
	};

	return ide_atapi_send(bdev, &rs, sizeof(rs), &rsd, sizeof(rsd));
}

static int ide_atapi_read_capacity(struct bdev *bdev)
{
	int ret;
	struct scsi_read_capacity10_data rcd;
	struct scsi_read_capacity10 rc = {
		.cmd = SCSI_CMD_READ_CAPACITY10
	};

	ret = ide_atapi_send(bdev, &rc, sizeof(rc), &rcd, sizeof(rcd));
	if (ide_atapi_request_sense(bdev))
		return -EFAULT;

	if (ret)
		return ret;

	bdev->last_block = betocpu32(rcd.last_block);
	bdev->block_size = betocpu32(rcd.block_size);

	return 0;
}

/*
 * IDE device operations: ATA
 */

static int ide_ata_read(struct bdev *device, uint64_t offset,
	uint64_t num, void *buffer)
{
	return 0;
}

static int ide_ata_write(struct bdev *device, uint64_t offset,
	uint64_t num, const void *buffer)
{
	return 0;
}

/*
 * IDE device operations: ATAPI
 */

static int ide_atapi_read(struct bdev *bdev, uint64_t offset,
	uint64_t num, void *buffer)
{
	int ret;
	struct scsi_xfer12 xf = {
		.cmd = SCSI_CMD_READ12,
		.lba = cputobe32(offset),
		.num = cputobe32(num)
	};

	ret = ide_atapi_send(bdev, &xf, sizeof(xf), buffer, num * bdev->block_size);
	if (ide_atapi_request_sense(bdev))
		return -EFAULT;

	if (ret)
		return ret;

	return 0;
}

static int ide_atapi_write(struct bdev *device, uint64_t offset,
	uint64_t num, const void *buffer)
{
	/*
	 * Not supported for this device.
	 */
	return -ENOTSUP;
}

/*
 * Struct for bdev_ops to be registered by bootloader
 */

static struct bdev_ops ide_ata_bdev_ops = {
	.read  = ide_ata_read,
	.write = ide_ata_write
};

static struct bdev_ops ide_atapi_bdev_ops = {
	.read  = ide_atapi_read,
	.write = ide_atapi_write
};

/*
 * Module initialization and exit function
 */

static int ide_dev_name(struct ide_dev *idedev, struct bdev *bdev)
{
	char ide[] = "ideX-Y";

	ide[3] = '0' + (idedev->channel << 1);
	ide[5] = '0' + (idedev->slave);
	bdev->name = bstrdup(ide);
	if (!bdev->name)
		return -ENOMEM;

	return 0;
}

static int ide_fill_ata(struct bdev *bdev)
{
	struct ide_dev *idedev = bdev->private;

	bdev->init_block = 0;

	if (!libata_has_lba_support(idedev->private))
		goto fill_ata_chs;

	bdev->flags |= BDEV_FLAGS_LBA;
	bdev->last_block = libata_last_block(idedev->private);

	goto fill_ata_block_size;

fill_ata_chs:

	bdev->cylinders = libata_cylinders(idedev->private);
	bdev->heads = libata_heads(idedev->private);
	bdev->sectors_per_track = libata_sectors_per_track(idedev->private);

fill_ata_block_size:

	bdev->block_size = libata_block_size(idedev->private);

#ifdef CONFIG_IDE_DEBUG
	bprintln(DRIVER_IDE ": %s: sectors = %llu, block size = %u",
		bdev->name, bdev->last_block, bdev->block_size);
#endif

	return 0;
}

static int ide_init_ata(struct ide_dev *idedev)
{
	struct bdev *bdev;

	idedev->private = bmalloc(ATA_IDENTIFY_SIZE);
	if (!idedev->private)
		return -ENOMEM;

	ide_read_io(idedev, idedev->private, ATA_IDENTIFY_SIZE);

	bdev = bmalloc(sizeof(*bdev));
	if (!bdev)
		goto ide_free_ibuf_ata;

	bdev->init_block = 0;

	if (ide_dev_name(idedev, bdev))
		goto ide_free_bdev_ata;

	idedev->disk = 0xe0;
	bdev->private = idedev;

	if (ide_fill_ata(bdev))
		goto ide_free_bdev_ata;

	return bdev_init(bdev, &ide_ata_bdev_ops);

ide_free_bdev_ata:
	bfree(bdev);

ide_free_ibuf_ata:
	bfree(idedev->private);

	return -EFAULT;
}

static int ide_fill_atapi(struct bdev *bdev)
{
	if (ide_atapi_read_capacity(bdev))
		return -EFAULT;

	bdev->flags |= BDEV_FLAGS_LBA;

#ifdef CONFIG_IDE_DEBUG
	bprintln(DRIVER_IDE ": %s: sectors = %llu, block size = %u",
		bdev->name, bdev->last_block, bdev->block_size);
#endif

	return 0;
}

static int ide_init_atapi(struct ide_dev *idedev)
{
	struct bdev *bdev = bmalloc(sizeof(*bdev));

	if (!bdev)
		goto ide_free_ibuf_atapi;

	if (ide_dev_name(idedev, bdev))
		goto ide_free_bdev_atapi;

	idedev->disk = 0;
	bdev->private = idedev;

	if (ide_fill_atapi(bdev))
		goto ide_free_bdev_atapi;

	return bdev_init(bdev, &ide_atapi_bdev_ops);

ide_free_bdev_atapi:
	bfree(bdev);

ide_free_ibuf_atapi:
	bfree(idedev->private);

	return -EFAULT;
}

static int ide_identify(struct ide_dev *idedev)
{
	int ret;
	uint8_t status;

	ide_outb(idedev, ATA_REG_HDDEVSEL, idedev->slave << 4);
	ide_outb(idedev, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);

	status = ide_poll_io(idedev);
	if (!status)
		return -ENODEV;

	if ((status & ATA_SR_POLL) == ATA_SR_ERR) {
		ret = ide_init_atapi(idedev);
		if (!ret)
			return 0;
	}

	return ide_init_ata(idedev);
}

static int ide_init_devchn(struct pci_dev *pcidev, uint16_t chn, int slave)
{
	struct ide_dev *idedev = bmalloc(sizeof(*idedev));

	if (!idedev)
		return -ENOMEM;

	idedev->io_base = ide_ports[chn];
	idedev->control = ide_ports[chn] + IDE_CTRL_OFFSET;

	idedev->channel = chn;
	idedev->slave = slave;
	idedev->bmaster = pcidev->bar[4];
	if (idedev->bmaster & 0x1)
		idedev->bmaster &= PCI_BAR_MASK;

	return ide_identify(idedev);
}

static int ide_init_controller(struct pci_dev *pcidev)
{
	uint16_t chn;

	for (chn = 0; chn < IDE_MAX_CHANNELS; chn++) {

		/*
		 * Initialize both master and slave device for channel 'chn'. We use
		 * the base address registers to obtain the io_base and controls values
		 * to initialize the PCI IDE controller.
		 */
		ide_init_devchn(pcidev, chn, 0);
		ide_init_devchn(pcidev, chn, 1);
	}

	return 0;
}

static int ide_init(void)
{
	struct pci_dev *pcidev;

	bprintln(DRIVER_IDE ": Initialize module...");

	/*
	 * TODO CRO: Iterate over all class-subclass combinations. The used
	 * IDE cotroller might not have the used class in the PCI_CLASS_IDE
	 * macro (it could be a different interface, like PCI).
	 */
	pcidev = pci_get_class(PCI_CLASS_IDE, NULL);
	if (!pcidev)
		return 0;

	return ide_init_controller(pcidev);
}

static void ide_exit(void)
{
	/*
	 * Not supported.
	 */

	bprintln(DRIVER_IDE ": Exit module...");
}

module_init(ide_init);
module_exit(ide_exit);
