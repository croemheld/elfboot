#include <asm/boot.h>
#include <asm/bios.h>
#include <asm/video.h>

#include <uapi/asm/bootparam.h>

#include <uapi/elfboot/common.h>

// static struct screen boot_screen = {
// 	.name = "VGA",
// 	.width = 80,
// 	.height = 25,
// 	.xunit = 1, 
// 	.yunit = 1, 
// 	.bpu = sizeof(uint16_t),
// 	.fgcolor.vga = SCREEN_COLOR_VGA(SCREEN_COLOR_VGA_WHITE), 
// 	.bgcolor.vga = SCREEN_COLOR_VGA(SCREEN_COLOR_VGA_BLACK), 
// 	.fbaddr = VGA_FRAMEBUFFER_ADDRESS
// };

static int video_vesa_probe(struct boot_params *boot_params)
{
	struct biosregs ireg, oreg;

	/*
	 * Get VBE information
	 */

	boot_params->vesa_info.sigvalue = VBE2;

	initregs(&ireg);
	ireg.ax = 0x4f00;
	ireg.di = tuint(&boot_params->vesa_info);

	bioscall(0x10, &ireg, &oreg);

	if (oreg.ax != 0x004f)
		return -ENOTSUP;

	return 0;
}

static int video_vesa_mode(struct boot_params *boot_params)
{
	struct biosregs ireg, oreg;

	/*
	 * Get current VESA mode
	 */

	initregs(&ireg);
	ireg.ax = 0x4f03;

	bioscall(0x10, &ireg, &oreg);

	/*
	 * Get VBE mode information
	 */

	initregs(&ireg);
	ireg.ax = 0x4f01;
	ireg.cx = 0x4000 + oreg.bx;
	ireg.di = tuint(&boot_params->vesa_mode);

	bioscall(0x10, &ireg, &oreg);

	if (oreg.ax != 0x004f)
		return -EFAULT;

	return 0;
}

static int video_inquire_vesa(struct boot_params *boot_params)
{
	int ret;

	/*
	 * Check if VBE extensions present
	 */

	ret = video_vesa_probe(boot_params);
	if (ret)
		return -EFAULT;

	/*
	 * Get current mode information
	 */

	ret = video_vesa_mode(boot_params);
	if (ret)
		return -EFAULT;

	return 0;
}

static int video_default_mode(void)
{
	struct biosregs ireg, oreg;

	/*
	 * Get current video mode
	 */

	initregs(&ireg);
	ireg.ah = 0x0f;

	bioscall(0x10, &ireg, &oreg);

	/*
	 * Reset current video mode and clear screen
	 */

	initregs(&ireg);
	ireg.al = oreg.al;

	bioscall(0x10, &ireg, &oreg);

	/*
	 * Turn off blinking (bit 7 of attribute byte
	 * in VGA byte-pair scheme)
	 */

	initregs(&ireg);
	ireg.ax = 0x1003;
	ireg.bl = 0x00;
	ireg.bh = 0x00;

	bioscall(0x10, &ireg, &oreg);

	return 0;
}

int detect_videos(struct boot_params *boot_params)
{
	int ret;

	/* Check if VESA modes available */
	// ret = video_inquire_vesa(boot_params);
	// if (ret)
	// 	return -EFAULT;

	/* Reset current video mode */
	ret = video_default_mode();
	if (ret)
		return -EFAULT;

	return 0;
}