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

#include <drivers/ata.h>
#include <drivers/scsi.h>
#include <drivers/ahci.h>

static int ahci_find_slot(volatile struct hba_port *port)
{
	int i;

	for (i = 0; i < AHCI_MAX_DEVICES; i++) {
		if ((port->sact | port->ci) & (1UL << i))
			continue;

		return i;
	}

	return -EFAULT;
}

static void ahci_port_wait(volatile struct hba_port *port)
{
	while (true) {
		if ((port->tfd & ATA_SR_BSY) || (port->tfd & ATA_SR_DRQ))
			continue;

		break;
	}
}

static void ahci_start_command(volatile struct hba_port *port)
{
	port->cmd &= ~HBA_PxCMD_ST;

	while (port->cmd & HBA_PxCMD_CR);

	port->cmd |= HBA_PxCMD_FRE;
	port->cmd |= HBA_PxCMD_ST;
}

static void ahci_stop_command(volatile struct hba_port *port)
{
	port->cmd &= ~HBA_PxCMD_ST;

	while (port->cmd & HBA_PxCMD_CR);

	port->cmd &= ~HBA_PxCMD_FRE;
}

static uint8_t ahci_is_atapi(volatile struct hba_port *port)
{
	switch (port->sig) {
		case SATA_SIG_ATAPI:
			return 1;
		default:
			return 0;
	}
}

static int ahci_port_rebase_headers(struct hba_cmd_hdr *hdr)
{
	int slot;

	for (slot = 0; slot < AHCI_MAX_DEVICES; slot++) {
		hdr[slot].prdtl = 8;
		hdr[slot].ctba = tuint(bzalloc(256));
		if (!hdr[slot].ctba)
			return -EFAULT;
	}

	return 0;
}

static int ahci_port_rebase(volatile struct hba_port *port)
{
	struct hba_cmd_hdr *hdr;

	ahci_stop_command(port);

	port->clb = tuint(bzalloc(1024));
	if (!port->clb)
		return -ENOMEM;
	port->clbu = 0;

	port->fb = tuint(bzalloc(256));
	if (!port->fb)
		goto ahci_port_free_clb;
	port->fbu = 0;

	hdr = tvptr(port->clb);
	if (ahci_port_rebase_headers(hdr))
		goto ahci_port_free_fb;

	ahci_start_command(port);

	return 0;

ahci_port_free_fb:
	bfree(tvptr(port->fb));

ahci_port_free_clb:
	bfree(tvptr(port->clb));

	return -EFAULT;
}

static struct hba_cmd_tbl *ahci_get_cmd_tbl(volatile struct hba_port *port, int slot)
{
	struct hba_cmd_hdr *chdr = vptradd(port->clb, slot * sizeof(*chdr));

	return tvptr(chdr->ctba);	
}

static struct hba_fis_reg *ahci_get_fis_reg(volatile struct hba_port *port, int slot)
{
	struct hba_cmd_tbl *ctbl = ahci_get_cmd_tbl(port, slot);

	return (struct hba_fis_reg *)ctbl->cfis;
}

static int ahci_prepare_command(struct ahci_dev *ahcidev, size_t num_prdts, int write)
{
	struct hba_cmd_hdr *chdr;
	struct hba_cmd_tbl *ctbl;
	struct hba_fis_reg *freg;
	int slot;

	slot = ahci_find_slot(ahcidev->port);
	if (slot < 0)
		return -EFAULT;

	ctbl = bzalloc(sizeof(*ctbl) + sizeof(struct hba_prdt) * num_prdts);
	if (!ctbl)
		return -ENOMEM;

	chdr = vptradd(ahcidev->port->clb, slot * sizeof(*chdr));
	chdr->pwacfl = HBA_PWACFL(5, ahcidev->atapi, write, 0);
	chdr->ctba = tuint(ctbl);
	chdr->ctbau = 0;
	chdr->prdtl = num_prdts;

	freg = ahci_get_fis_reg(ahcidev->port, slot);
	freg->type = FIS_TYPE_REG_H2D;
	freg->pmic = HBA_PMIC(0, 0, 1);
	freg->control = 0x08;

	return slot;
}

static void ahci_destroy_command(volatile struct hba_port *port, int slot)
{
	struct hba_cmd_hdr *chdr = vptradd(port->clb, slot * sizeof(*chdr));

	bfree(tvptr(chdr->ctba));
}

/*
 * AHCI device operations
 */

static int ahci_idle(struct ahci_dev *ahcidev, int slot)
{
	while (ahcidev->port->ci & (1UL << slot)) {
		if (ahcidev->port->is & HBA_PxIS_TFES)
			return -EFAULT;
	}

	if (ahcidev->port->is & HBA_PxIS_TFES)
		return -EFAULT;

	return 0;
}

static int ahci_send_command(struct ahci_dev *ahcidev, struct ahci_cmd *cmd)
{
	struct hba_fis_reg *freg;
	struct hba_cmd_tbl *ctbl;
	int slot, ret = 0;

	slot = ahci_prepare_command(ahcidev, 1, 0);
	if (slot < 0) {
		bprintln(DRIVER_AHCI ": Error preparing command: %d", slot);
		return slot;
	}

	freg = ahci_get_fis_reg(ahcidev->port, slot);
	freg->command = ATA_CMD_PACKET;
	freg->device  = 0xA0 | (1 << 6);
	freg->feature = 0x01;

	ctbl = ahci_get_cmd_tbl(ahcidev->port, slot);
	ctbl->prdt[0].idbc = HBA_IDBC(cmd->buflen - 1, 0);
	ctbl->prdt[0].dba  = tuint(cmd->buffer);

	memcpy(ctbl->acmd, cmd->packet, cmd->pkglen);

	ahci_port_wait(ahcidev->port);
	ahcidev->port->ci |= (1UL << slot);

	if (ahci_idle(ahcidev, slot))
		ret = -EFAULT;

	ahci_destroy_command(ahcidev->port, slot);

	return ret;
}

static int ahci_satapi_send(struct bdev *bdev, void *packet, size_t pkglen,
	void *buffer, size_t buflen)
{
	struct ahci_cmd cmd = { 0 };

	cmd.packet = packet;
	cmd.pkglen = pkglen;
	cmd.buffer = buffer;
	cmd.buflen = buflen;

	return ahci_send_command(bdev->private, &cmd);
}

/*
 * AHCI device operations: SATA
 */

static int ahci_sata_read(struct bdev *bdev, uint64_t offset, uint64_t num,
	void *buffer)
{
	return 0;
}

static int ahci_sata_write(struct bdev *bdev, uint64_t offset, uint64_t num,
	const void *buffer)
{
	return 0;
}

static int ahci_sata_ioctl(struct bdev *bdev, int request, void *args)
{
	return 0;
}

/*
 * AHCI device operations: SATAPI
 */

static int ahci_satapi_request_sense(struct bdev *bdev)
{
	struct scsi_request_sense_data rsd;
	struct scsi_request_sense rs = {
		.cmd = SCSI_CMD_REQUEST_SENSE,
		.len = sizeof(rs)
	};

	return ahci_satapi_send(bdev, &rs, sizeof(rs), &rsd, sizeof(rsd));
}

static int ahci_satapi_read(struct bdev *bdev, uint64_t offset, uint64_t num,
	void *buffer)
{
	int ret;
	struct scsi_xfer12 xf = {
		.cmd = SCSI_CMD_READ12,
		.lba = cputobe32(offset),
		.num = cputobe32(num)
	};

	ret = ahci_satapi_send(bdev, &xf, sizeof(xf), buffer, num * bdev->block_size);
	if (ahci_satapi_request_sense(bdev))
		return -EFAULT;

	if (ret)
		return ret;

	return 0;
}

static int ahci_satapi_write(struct bdev *bdev, uint64_t offset, uint64_t num,
	const void *buffer)
{
	return 0;
}

static int ahci_satapi_ioctl(struct bdev *bdev, int request, void *args)
{
	return 0;
}

/*
 * Struct for bdev_ops to be registered by bootloader
 */

static struct bdev_ops ahci_sata_bdev_ops = {
	.read  = ahci_sata_read,
	.write = ahci_sata_write,
	.ioctl = ahci_sata_ioctl
};

static struct bdev_ops ahci_satapi_bdev_ops = {
	.read  = ahci_satapi_read,
	.write = ahci_satapi_write,
	.ioctl = ahci_satapi_ioctl
};

/*
 * Module initialization and exit function
 */

static int ahci_dev_name(struct ahci_dev *ahcidev, struct bdev *bdev)
{
	char ahci[] = "ahciXX";

	if (ahcidev->portno < 10) {
		ahci[4] = '0' + (ahcidev->portno);
		ahci[5] =  0;
	} else {
		ahci[4] = '0' + (ahcidev->portno / 10);
		ahci[5] = '0' + (ahcidev->portno % 10);
	}

	bdev->name = bstrdup(ahci);
	if (!bdev->name)
		return -ENOMEM;

	return 0;
}

static int ahci_fill_sata(struct ahci_dev *ahcidev)
{
	struct bdev *bdev;

	bdev = bmalloc(sizeof(*bdev));
	if (!bdev)
		return -ENOMEM;

	if (ahci_dev_name(ahcidev, bdev))
		goto ahci_sata_free_bdev;

	bdev->init_block = 0;
	bdev->private = ahcidev;

	if (!libata_has_lba_support(ahcidev->private))
		goto fill_ata_chs;

	bdev->flags |= BDEV_FLAGS_LBA;
	bdev->last_block = libata_last_block(ahcidev->private);

	goto fill_ata_block_size;

fill_ata_chs:

	bdev->cylinders = libata_cylinders(ahcidev->private);
	bdev->heads = libata_heads(ahcidev->private);
	bdev->sectors_per_track = libata_sectors_per_track(ahcidev->private);

fill_ata_block_size:

	bdev->block_size = libata_block_size(ahcidev->private);

#ifdef CONFIG_DRIVER_AHCI_DEBUG
	bprintln(DRIVER_AHCI ": %s: sectors = %llu, block size = %u",
		bdev->name, bdev->last_block, bdev->block_size);
#endif

	return bdev_init(bdev, &ahci_sata_bdev_ops);

ahci_sata_free_bdev:
	bfree(bdev);

	return -EFAULT;
}

static int ahci_satapi_read_capacity(struct bdev *bdev)
{
	int ret;
	struct scsi_read_capacity10_data rcd;
	struct scsi_read_capacity10 rc = {
		.cmd = SCSI_CMD_READ_CAPACITY10
	};

	ret = ahci_satapi_send(bdev, &rc, sizeof(rc), &rcd, sizeof(rcd));
	if (ahci_satapi_request_sense(bdev))
		return -EFAULT;

	if (ret)
		return ret;

	bdev->last_block = betocpu32(rcd.last_block);
	bdev->block_size = betocpu32(rcd.block_size);

	return 0;
}

static int ahci_fill_satapi(struct ahci_dev *ahcidev)
{
	struct bdev *bdev;

	ahcidev->atapi = 1;

	bdev = bmalloc(sizeof(*bdev));
	if (!bdev)
		return -ENOMEM;

	if (ahci_dev_name(ahcidev, bdev))
		goto ahci_satapi_free_bdev;

	bdev->flags |= BDEV_FLAGS_LBA;
	bdev->private = ahcidev;

	bdev->init_block = 0;

	if (ahci_satapi_read_capacity(bdev))
		goto ahci_satapi_free_bdev;

#ifdef CONFIG_DRIVER_AHCI_DEBUG
	bprintln(DRIVER_AHCI ": %s: sectors = %llu, block size = %u",
		bdev->name, bdev->last_block, bdev->block_size);
#endif

	return bdev_init(bdev, &ahci_satapi_bdev_ops);

ahci_satapi_free_bdev:
	bfree(bdev);

	return -EFAULT;
}

static int ahci_identify_ahcidev(struct ahci_dev *ahcidev)
{
	struct hba_cmd_tbl *ctbl;
	struct hba_fis_reg *freg;
	int slot, ret;
	uint8_t command;

	slot = ahci_prepare_command(ahcidev, 1, 0);
	if (slot < 0)
		return -EFAULT;

	if (ahci_is_atapi(ahcidev->port))
		command = ATA_CMD_IDENTIFY_PACKET;
	else
		command = ATA_CMD_IDENTIFY;

	freg = ahci_get_fis_reg(ahcidev->port, slot);
	freg->command = command;
	freg->device = 0xA0;

	ctbl = ahci_get_cmd_tbl(ahcidev->port, slot);
	ctbl->prdt[0].idbc = HBA_IDBC(ATA_IDENTIFY_SIZE, 0);
	ctbl->prdt[0].dba = tuint(ahcidev->private);

	ahci_port_wait(ahcidev->port);
	ahcidev->port->ci |= (1UL << slot);

	if (ahci_idle(ahcidev, slot)) {
		ret = -EFAULT;
		goto ahci_identify_destroy_command;
	}

	switch (ahcidev->port->sig) {
		case SATA_SIG_ATA:
			ret = ahci_fill_sata(ahcidev);
			break;
		case SATA_SIG_ATAPI:
			ret = ahci_fill_satapi(ahcidev);
			break;
		default:
			ret = 0;
	}

ahci_identify_destroy_command:
	ahci_destroy_command(ahcidev->port, slot);

	return ret;
}

static int ahci_init_ahcidev(struct ahci_dev *ahcidev,
	volatile struct hba_port *port)
{
	int ret;

	if (ahci_port_rebase(port))
		return -EFAULT;

	ahcidev->private = bmalloc(ATA_IDENTIFY_SIZE);
	if (!ahcidev->private)
		return -ENOMEM;

	ahcidev->port = port;

	ret = ahci_identify_ahcidev(ahcidev);

	bfree(ahcidev->private);

	return ret;
}

static int ahci_init_port(struct pci_dev *pcidev, int port)
{
	struct ahci_dev *ahcidev;
	struct hba_memory *hba_mem;

	ahcidev = bmalloc(sizeof(*ahcidev));
	if (!ahcidev)
		return -ENOMEM;

	ahcidev->portno = port;

	hba_mem = tvptr(pcidev->bar[5]);
	if (ahci_init_ahcidev(ahcidev, &hba_mem->ports[port]))
		goto ahci_init_free_ahcidev;

	return 0;

ahci_init_free_ahcidev:
	bfree(ahcidev);

	return -EFAULT;
}

static int ahci_check_type(struct hba_memory *hba, int port)
{
	uint8_t det, ipm;

	det = (hba->ports[port].ssts >> 0) & 0x0f;
	ipm = (hba->ports[port].ssts >> 8) & 0x0f;

	/*
	 * We only accept devices with the following state:
	 *
	 * det: Phy communication established
	 * ipm: AHCI device is in active state
	 */
	if ((det != 0x03) || (ipm != 0x01))
		return -ENODEV;

	return 0;
}

static int ahci_probe_port(struct pci_dev *pcidev, int port)
{
	struct hba_memory *mem = tvptr(pcidev->bar[5]);

	if (!(mem->pi & (1UL << port)))
		return -ENODEV;

	return ahci_check_type(mem, port);
}

static int ahci_init_controller(struct pci_dev *pcidev, struct hba_memory *hba_memory)
{
	int port, num_dports;

	pci_set_master(pcidev);

	num_dports = HBA_CAP_NP(hba_memory->cap) + 1;
	for (port = 0; port < num_dports; port++) {

		/*
		 * Check the device signature. We ignore non-existent devices and only
		 * initialize devices that are active and have a phy comm established.
		 */
		if (ahci_probe_port(pcidev, port))
			continue;

		ahci_init_port(pcidev, port);
	}

	return 0;
}

static int ahci_init(void)
{
	struct pci_dev *pcidev;

	bprintln(DRIVER_AHCI ": Initialize module...");

	/*
	 * TODO CRO: Use table of allowed PCI devices
	 */

	pcidev = pci_get_class(PCI_CLASS_SATA, NULL);
	if (!pcidev) {
		pcidev = pci_get_class(PCI_CLASS_RAID, NULL);

		if (!pcidev)
			return 0;
	}

	return ahci_init_controller(pcidev, tvptr(pcidev->bar[5]));
}

static void ahci_exit(void)
{
	/*
	 * Not supported.
	 */

	bprintln(DRIVER_AHCI ": Exit module...");
}

module_init(ahci_init);
module_exit(ahci_exit);
