#ifndef __DRIVER_VGA_H__
#define __DRIVER_VGA_H__

#define VGA_FRAMEBUFFER_ADDRESS		0xB8000



void vga_driver_init(void);

void vga_driver_fini(void);

#endif /* __DRIVER_VGA_H__ */