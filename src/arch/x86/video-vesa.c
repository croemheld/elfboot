#include <elfboot/core.h>
#include <elfboot/mm.h>
#include <elfboot/screen.h>
#include <elfboot/console.h>
#include <elfboot/printf.h>

#include <asm/bios.h>
#include <asm/video.h>

#include <uapi/elfboot/common.h>

static uint16_t *vesa_modes;

void vesa_mode_dump(uint16_t mode, struct vesa_mode *vesa_mode)
{
	bprintln("VESA mode: %lx", mode);
	bprintln("Resolution: %lux%lu", vesa_mode->width, vesa_mode->height);
	bprintln("Char resolution: %lux%lu", vesa_mode->w_char, vesa_mode->y_char);
	bprintln("BPP: %lu", vesa_mode->bpp);
	bprintln("Framebuffer: %p", vesa_mode->framebuffer);

	asm volatile("xchg %bx, %bx");
}

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

	bprintln("Check vesa mode %lx", mode);

	initregs(&ireg);
	ireg.ax = 0x4f01;

	/* Request linear framebuffer */
	ireg.cx = 0x4000 + mode;
	ireg.di = vptrtuint(vesa_mode);

	bioscall(0x10, &ireg, &oreg);

	if (oreg.al != 0x4f) {
		bprintln("oreg.al");
		return -EFAULT;
	}

	if (oreg.ah != 0x00) {
		bprintln("oreg.ah");
		return -EFAULT;
	}

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

void detect_videos_vesa(void)
{
	int i = 0, j;
	uint16_t *mode;
	struct vesa_info vesa_info;
	struct vesa_mode vesa_mode;

	if (get_vesa_info(&vesa_info) < 0) {
		bprintln("Error in get_vesa_info");
		return;
	}

	while(1) {
		mode = vptradd(uinttvptr(vesa_info.video_modes), i * sizeof(uint16_t));

		if (*mode == 0xffff)
			break;

		i++;
	}

	vesa_modes = bmalloc(i * sizeof(uint16_t));
	if (!vesa_modes) {
		bprintln("Error in bmalloc");
		return;
	}

	memcpy(vesa_modes, uinttvptr(vesa_info.video_modes), i * sizeof(uint16_t));

	for (j = 0; j < i; j++) {
		if (get_vesa_mode(vesa_modes[j], &vesa_mode) < 0) {
			bprintln("Error in get_vesa_mode");
			return;
		}

		vesa_mode_dump(vesa_modes[j], &vesa_mode);
	}
}