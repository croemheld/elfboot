#include <boot/boot.h>

struct edid_data edid_data;

struct vesa_info vesa_info = {
	.signature = "VBE2"
};

static int read_edid(void)
{
	struct biosregs ireg, oreg;

	initregs(&ireg);
	ireg.ax = 0x4f15;
	ireg.bl = 0x01;
	ireg.di = vptrtuint(&edid_data);

	bioscall(0x10, &ireg, &oreg);

	if(oreg.al != 0x4f)
		return 0;

	if(oreg.ah != 0x00)
		return 0;

	return 1;
}

static int get_vesa_info(void)
{
	struct biosregs ireg, oreg;

	initregs(&ireg);
	ireg.ax = 0x4f00;
	ireg.di = vptrtuint(&vesa_info);

	__asm__ volatile("xchg %bx, %bx");

	bioscall(0x10, &ireg, &oreg);

	if(oreg.al != 0x4f)
		return 0;

	if(oreg.ah != 0x00)
		return 0;

	return 1;	
}

void detect_videos(struct boot_params *boot_params)
{
	uint32_t vbe_width = 0, vbe_height = 0;

	if(cmdline_get_boolean_value("enable_edid")) {
		if(read_edid()) {

		} else
			bprintf("Could not read EDID information!\n");
	}

	if(!vbe_width || !vbe_height) {
		vbe_width  = cmdline_get_int_value("screen_width");
		vbe_height = cmdline_get_int_value("screen_height");
	}

	if(!get_vesa_info()) {
		bprintf("Could not read VESA information!\n");

		return;
	}

	bprintf("Address to EDID structure: %p\n", &edid_data);
	bprintf("Address to VESA structure: %p\n", &vesa_info);
}