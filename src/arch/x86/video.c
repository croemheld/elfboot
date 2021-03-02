#include <asm/boot.h>
#include <asm/bios.h>
#include <asm/video.h>

#include <uapi/asm/bootparam.h>

#include <uapi/elfboot/common.h>

// static int video_vesa_probe(struct boot_params *boot_params)
// {
// 	struct biosregs ireg, oreg;
// 
// 	/*
// 	 * Get VBE information
// 	 */
// 
// 	boot_params->vesa_info.sigvalue = VBE2;
// 
// 	initregs(&ireg);
// 	ireg.ax = 0x4f00;
// 	ireg.di = tuint(&boot_params->vesa_info);
// 
// 	bioscall(0x10, &ireg, &oreg);
// 
// 	if (oreg.ax != 0x004f)
// 		return -ENOTSUP;
// 
// 	return 0;
// }
// 
// static int video_vesa_mode(struct boot_params *boot_params)
// {
// 	struct biosregs ireg, oreg;
// 
// 	/*
// 	 * Get current VESA mode
// 	 */
// 
// 	initregs(&ireg);
// 	ireg.ax = 0x4f03;
// 
// 	bioscall(0x10, &ireg, &oreg);
// 
// 	/*
// 	 * Get VBE mode information
// 	 */
// 
// 	initregs(&ireg);
// 	ireg.ax = 0x4f01;
// 	ireg.cx = 0x4000 + oreg.bx;
// 	ireg.di = tuint(&boot_params->vesa_mode);
// 
// 	bioscall(0x10, &ireg, &oreg);
// 
// 	if (oreg.ax != 0x004f)
// 		return -EFAULT;
// 
// 	return 0;
// }
// 
// static int video_inquire_vesa(struct boot_params *boot_params)
// {
// 	int ret;
// 
// 	/*
// 	 * Check if VBE extensions present
// 	 */
// 
// 	ret = video_vesa_probe(boot_params);
// 	if (ret)
// 		return -EFAULT;
// 
// 	/*
// 	 * Get current mode information
// 	 */
// 
// 	ret = video_vesa_mode(boot_params);
// 	if (ret)
// 		return -EFAULT;
// 
// 	return 0;
// }

static int video_default_mode(void)
{
	struct biosregs ireg, oreg;

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
	/* Check if VESA modes available */
	// video_inquire_vesa(boot_params);

	/* Reset current video mode */
	return video_default_mode();
}