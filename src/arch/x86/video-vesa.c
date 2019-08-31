#include <elfboot/core.h>
#include <elfboot/mm.h>
#include <elfboot/screen.h>
#include <elfboot/console.h>
#include <elfboot/printf.h>

#include <asm/bios.h>
#include <asm/video.h>

#include <uapi/elfboot/common.h>

int get_vesa_info(struct vesa_info *vesa_info)
{
	struct biosregs ireg, oreg;

	initregs(&ireg);
	ireg.ax = 0x4f00;
	ireg.di = vptrtuint(vesa_info);

	bioscall(0x10, &ireg, &oreg);

	if (oreg.al != 0x4f)
		return -EFAULT;

	if (oreg.ah != 0x00)
		return -EFAULT;

	return 0;	
}

int get_vesa_mode(uint16_t mode, struct vesa_mode *vesa_mode)
{
	struct biosregs ireg, oreg;

	initregs(&ireg);
	ireg.ax = 0x4f01;

	/* Request linear framebuffer */
	ireg.cx = 0x4000 + mode;
	ireg.di = vptrtuint(vesa_mode);

	bioscall(0x10, &ireg, &oreg);

	if (oreg.al != 0x4f)
		return -EFAULT;

	if (oreg.ah != 0x00)
		return -EFAULT;

	return 0;
}

int set_vesa_mode(uint16_t mode)
{
	struct biosregs ireg, oreg;

	initregs(&ireg);
	ireg.ax = 0x4f02;

	/* Use linear framebuffer */
	ireg.bx = 0x4000 + mode;

	bioscall(0x10, &ireg, &oreg);

	if (oreg.al != 0x4f)
		return -EFAULT;

	if (oreg.ah != 0x00)
		return -EFAULT;

	return 0;
}

uint16_t get_current_vesa_mode(void)
{
	struct biosregs ireg, oreg;

	initregs(&ireg);
	ireg.ax = 0x4f03;

	bioscall(0x10, &ireg, &oreg);

	if (oreg.al != 0x4f)
		return -EFAULT;

	if (oreg.ah != 0x00)
		return -EFAULT;

	return oreg.bx;
}