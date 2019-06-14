#include <elfboot/core.h>
#include <elfboot/screen.h>
#include <elfboot/console.h>

#include <drivers/vga.h>

#include <asm/bios.h>
#include <asm/video.h>

#include <uapi/elfboot/common.h>

#include <uapi/asm/bootparam.h>

static struct screen boot_screen = {
	.name = "VGA",
	.width = 80,
	.height = 25,
	.xunit = 1, 
	.yunit = 1, 
	.bpu = sizeof(uint16_t),
	.fgcolor.vga = SCREEN_COLOR_VGA(SCREEN_COLOR_VGA_WHITE), 
	.bgcolor.vga = SCREEN_COLOR_VGA(SCREEN_COLOR_VGA_BLACK), 
	.fbaddr = VGA_FRAMEBUFFER_ADDRESS
};

static int video_default_mode(void)
{
	struct biosregs ireg, oreg;

	initregs(&ireg);
	ireg.ah = 0x0f;

	bioscall(0x10, &ireg, &oreg);

	initregs(&ireg);
	ireg.al = oreg.al;

	bioscall(0x10, &ireg, &oreg);

	return oreg.al;
}

void detect_videos(struct boot_params *boot_params)
{
	(void)boot_params;

	video_default_mode();

	console_init(&boot_screen);
}