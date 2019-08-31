#ifndef __X86_BOOT_H__
#define __X86_BOOT_H__

#define IMAGE_ADDR                                0x7C00
#define STACK_ADDR_START                          0x2000
#define STACK_ADDR_END                            0x1000
#define STACK_SIZE	                     (STACK_ADDR_START - STACK_ADDR_END)

#define SETUP_PHYS_ADDR                           0x8000
#define SETUP_SEGMENT                             0x07E0

#define VGA_FONT_BITMAP_ADDRESS                   0xA000
#define CMDLINE_BUFFER_ADDRESS                    0xB000

#define REALMODE_ADDRESS_LIMIT                    0xFFFF

#define IMG_ADDRESS                               0x7C00
#define IMG_MAX_ADDRESS                           0x00010000
#define IMG_MAX_SIZE                             (IMG_MAX_ADDRESS - IMG_ADDRESS)

#define IVT_ADDRESS                               0x0000
#define IVT_MAX_SIZE                              0x0400

#ifndef __ASSEMBLER__

#include <elfboot/core.h>
#include <elfboot/linkage.h>

#include <uapi/asm/bootparam.h>

/* Retrieve e820 memory map */
void detect_memory(struct boot_params *boot_params);

/* Detect supported video modes */
void detect_videos(struct boot_params *boot_params);

/* Load kernel */
void prepare_kernel(struct boot_params *boot_params);

/* Jump into our cr0S kernel */
void kernel_init(void);

#endif /* __ASSEMBLER__ */

#endif /* __X86_BOOT_H__ */