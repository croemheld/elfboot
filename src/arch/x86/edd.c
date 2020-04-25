#include <elfboot/core.h>
#include <elfboot/mm.h>
#include <elfboot/sections.h>
#include <elfboot/printf.h>

#include <asm/bios.h>
#include <asm/edd.h>

#include <uapi/asm/bootparam.h>

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

	return 0;
}

static int edd_read_device_params(uint8_t devno, struct edd_device_params *edp)
{
	struct biosregs ireg, oreg;
	struct edd_device_params ebuf;

	initregs(&ireg);
	ireg.ah = 0x48;
	ireg.dl = devno;
	ireg.si = tuint(&ebuf);

	ebuf.length = sizeof(*edp);

	bioscall(0x13, &ireg, &oreg);

	if (oreg.eflags & X86_EFLAGS_CF)
		return oreg.ah;

	memcpy(edp, &ebuf, sizeof(*edp));

	return 0;
}

int edd_read_device_info(uint8_t devno, struct edd_device_info *edi)
{
	/*
	 * Reading extended drive parameters.
	 *
	 * If the current device does not support LBA addressing, we 
	 * simply abort the initialization procedure since the device 
	 * must then support CHS addressing. For now at least.
	 */

	if (edd_extensions_present(devno, edi))
		return -ENODEV;

	if (edd_read_device_params(devno, &edi->params))
		return -ENODEV;

	return 0;
}

int edd_extended_read(struct edd_extfer_info *ext)
{
	struct biosregs ireg, oreg;

	struct disk_address_packet dap = {
		.len = sizeof(dap),
		.num = min(ext->num, (BUFFER_SIZE / ext->bps)),
		.buf = BUFFER_START,
		.lba = ext->lba,
	};

	initregs(&ireg);
	ireg.ah = 0x42;
	ireg.dl = ext->dev;
	ireg.si = tuint(&dap);

	while (ext->num) {
		bioscall(0x13, &ireg, &oreg);

		if (oreg.eflags & X86_EFLAGS_CF)
			return oreg.ah;

		memcpy(tvptr(ext->buf), tvptr(dap.buf), dap.num * ext->bps);

		/* Update ext header */
		ext->num -= dap.num;
		ext->buf += dap.num * ext->bps;

		/* Update DAP structure */
		dap.lba += dap.num;
		dap.buf += dap.num * ext->bps;
		dap.num  = min(ext->num, (BUFFER_SIZE / ext->bps));
	};

	return 0;
}

// static int edd_device_setup(struct device *device, struct edd_device_info *edi)
// {
// 	struct edd_disk_drive_params *ddp;
// 	const char *interface_type = edi->params.interface_type;
// 
// 	/*
// 	 * Physical devices require I/O ports and controls
// 	 * in order to communicate with the bootloader.
// 	 */
// 
// 	device->io = bmalloc(sizeof(*device->io));
// 	if (!device->io)
// 		return -EFAULT;
// 	
// 	/* Devices created here are always block devices */
// 	device->type = DEVICE_BLOCK;
// 
// 	/* Basic parameters for IO */
// 	ddp = segment_offset_ptr(edi->params.dpte_ptr);
// 	device->io->io_base = ddp->io_base;
// 	device->io->control = ddp->control;
// 
// 	/*
// 	 * All devices created by edd_device_create() are actual
// 	 * physical devices such as ATA or SCSI. Virtual devices
// 	 * such as TTY and streams do not rely on actual devices
// 	 * and are handled separately.
// 	 *
// 	 * For all devices created here, the DEVICE_FLAGS_VIRTUAL
// 	 * flag is cleared. Also, since we are using the BIOS EDD
// 	 * extensions, we know tha the device is a block device.
// 	 */
// 
// 	/* Determine device type via interface */
// 	if (!edd_device_is_type(interface_type, EDD_DEVICE_INTERFACE_ATAPI)) {
// 		device_set(device, DEVICE_FLAG_LUN);
// 		device->info.lun = edi->params.device_path.atapi.lun;
// 	}
// 
// 	/* Device is slave */
// 	if (ddp->flags & EDD_DISK_DRIVE_PARAM_SLAVE)
// 		device_set(device, DEVICE_FLAG_SLAVE);
// 
// 	/* Device uses LBA addressing */
// 	if (ddp->flags & EDD_DISK_DRIVE_PARAM_LBA)
// 		device_set(device, DEVICE_FLAG_LBA);
// 
// 	/* Device has valid CHS information */
// 	if (edi->params.info_flags & EDD_DEVICE_PARAM_CHS_VALID) {
// 		device_set(device, DEVICE_FLAG_CHS);
// 		device->info.cylinders = edi->params.num_cylinders;
// 		device->info.heads = edi->params.num_heads;
// 		device->info.sectors = edi->params.num_sectors;
// 		device->info.total_sectors = edi->params.total_sectors;
// 	}
// 
// 	/* Sector size is always valid */
// 	device->info.block_size = edi->params.bytes_per_sector;
// 
// 	return 0;
// }
// 
// struct device *edd_device_create(uint8_t devno)
// {
// 	struct edd_device_info edi;
// 	struct device *device;
// 
// 	if (edd_read_device_info(devno, &edi))
// 		return NULL;
// 
// 	device = bmalloc(sizeof(*device));
// 	if (!device)
// 		return NULL;
// 
// 	if (edd_device_setup(device, &edi)) {
// 		bfree(device);
// 		return NULL;
// 	}
// 
// 	return device;
// }
// 
// int edd_query_devices(void)
// {
// 	uint8_t devno;
// 	struct device *device;
// 	char name[] = "sda";
// 
// 	for (devno = 0x80; devno < 0x80 + EDD_MAX_DEVICES; devno++) {
// 		device = edd_device_create(devno);
// 
// 		if (!device)
// 			continue;
// 
// 		if (!device_create(device, name))
// 			continue;
// 
// 		/*
// 		 * Use sda, sdb, ... etc as names for our devices.
// 		 */
// 		
// 		name[2]++;
// 	}
// 
// 	return 0;
// }