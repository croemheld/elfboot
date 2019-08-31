#include <elfboot/core.h>
#include <elfboot/mm.h>
#include <elfboot/device.h>
#include <elfboot/printf.h>

#include <asm/bios.h>
#include <asm/edd.h>

#include <uapi/elfboot/common.h>

static void edd_read_legacy_params(uint8_t devno, struct edd_device_info *edi)
{
	struct biosregs ireg, oreg;

	initregs(&ireg);
	ireg.ah = 0x08;
	ireg.dl = devno;

	bioscall(0x13, &ireg, &oreg);

	if (oreg.eflags & X86_EFLAGS_CF)
		return;

	edi->legacy_max_cylinder = oreg.ch + ((oreg.cl & 0xc0) << 2);
	edi->legacy_max_head = oreg.dh;
	edi->legacy_sectors_per_track = oreg.cl & 0x3f;
}

static int edd_extensions_present(uint8_t devno, struct edd_device_info *edi)
{
	struct biosregs ireg, oreg;

	initregs(&ireg);
	ireg.ah = 0x41;
	ireg.bx = EDD_MAGIC1;
	ireg.dl = devno;

	bioscall(0x13, &ireg, &oreg);

	if (oreg.eflags & X86_EFLAGS_CF)
		return oreg.ah;

	if (oreg.bx != EDD_MAGIC2)
		return -1;

	edi->device = devno;
	edi->version = oreg.ah;
	edi->interface_support = oreg.cx;

	return 0;
}

static int edd_read_device_params(uint8_t devno, struct edd_device_params *edp)
{
	struct biosregs ireg, oreg;
	struct edd_device_params ebuf;

	initregs(&ireg);
	ireg.ah = 0x48;
	ireg.dl = devno;
	ireg.si = vptrtuint(&ebuf);

	ebuf.length = sizeof(*edp);

	bioscall(0x13, &ireg, &oreg);

	if (oreg.eflags & X86_EFLAGS_CF)
		return oreg.ah;

	memcpy(edp, &ebuf, sizeof(*edp));

	return 0;
}

static int edd_read_device_info(uint8_t devno, struct edd_device_info *edi)
{
	/*
	 * Reading legacy parameters.
	 *
	 * edd_read_legacy_params may fail depending on the underlying 
	 * device. However, the initialization should not be aborted 
	 * even if it fails. It just means the current drive does not 
	 * support old CHS addressing.
	 */
	
	edd_read_legacy_params(devno, edi);

	/*
	 * Reading extended drive parameters.
	 *
	 * If the current device does not support LBA addressing, we 
	 * simply abort the initialization procedure since the device 
	 * must then support CHS addressing.
	 */

	if (edd_extensions_present(devno, edi))
		return -1;

	if (edd_read_device_params(devno, &edi->params))
		return -1;

	return 0;
}

static int edd_device_setup(struct device *device, struct edd_device_info *edi)
{
	struct edd_disk_drive_params *ddp;
	const char *interface_type = edi->params.interface_type;

	/*
	 * Physical devices require I/O ports and controls
	 * in order to communicate with the bootloader.
	 */

	device->io = bmalloc(sizeof(*device->io));
	if (!device->io)
		return -EFAULT;
	
	/* Devices created here are always block devices */
	device->type = DEVICE_BLOCK;

	/* Basic parameters for IO */
	ddp = segment_offset_ptr(edi->params.dpte_ptr);
	device->io->io_base = ddp->io_base;
	device->io->control = ddp->control;

	/*
	 * All devices created by edd_device_create() are actual
	 * physical devices such as ATA or SCSI. Virtual devices
	 * such as TTY and streams do not rely on actual devices
	 * and are handled separately.
	 *
	 * For all devices created here, the DEVICE_FLAGS_VIRTUAL
	 * flag is cleared. Also, since we are using the BIOS EDD
	 * extensions, we know tha the device is a block device.
	 */

	/* Determine device type via interface */
	if (!edd_device_is_type(interface_type, EDD_DEVICE_INTERFACE_ATAPI)) {
		device_set(device, DEVICE_FLAG_LUN);
		device->info.lun = edi->params.device_path.atapi.lun;
	}

	/* Device is slave */
	if (ddp->flags & EDD_DISK_DRIVE_PARAM_SLAVE)
		device_set(device, DEVICE_FLAG_SLAVE);

	/* Device uses LBA addressing */
	if (ddp->flags & EDD_DISK_DRIVE_PARAM_LBA)
		device_set(device, DEVICE_FLAG_LBA);

	/* Device has valid CHS information */
	if (edi->params.info_flags & EDD_DEVICE_PARAM_CHS_VALID) {
		device_set(device, DEVICE_FLAG_CHS);
		device->info.cylinders = edi->params.num_cylinders;
		device->info.heads = edi->params.num_heads;
		device->info.sectors = edi->params.num_sectors;
		device->info.total_sectors = edi->params.total_sectors;
	}

	/* Sector size is always valid */
	device->info.block_size = edi->params.bytes_per_sector;

	return 0;
}

struct device *edd_device_create(uint8_t devno)
{
	struct edd_device_info edi;
	struct device *device;

	if (edd_read_device_info(devno, &edi))
		return NULL;

	device = bmalloc(sizeof(*device));
	if (!device)
		return NULL;

	if (edd_device_setup(device, &edi)) {
		bfree(device);
		return NULL;
	}

	return device;
}