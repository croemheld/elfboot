#include <boot/boot.h>
#include <boot/bios.h>
#include <boot/linkage.h>

void bmain(struct boot_params *boot_params)
{
	char *version;
	uint16_t devno;

	devno = boot_params->disk_drive;

	if(!iso_load_file(devno, CMDLINE_BUFFER_ADDRESS, "/CMDLINE.TXT;1"))
		return;

	version = cmdline_get_string_value("version");

	bprintf("Initializing cr0S bootloader (Version: %s)...\n", version);

	/* Retrieve e820 memory map */
	detect_memory(boot_params);

	/* Detect supported video modes */
	detect_videos(boot_params);

	/* Load kernel */
	// prepare_kernel(&boot_params);

	/* Jump into our cr0S kernel */
	// kernel_init();
}