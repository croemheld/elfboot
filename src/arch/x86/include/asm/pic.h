#ifndef __X86_PIC_H__
#define __X86_PIC_H__

#include <elfboot/core.h>

#define PIC_MASTER_CMD                            0x20
#define PIC_SLAVE_CMD                             0xA0

#define PIC_MASTER_DATA                           0x21
#define PIC_SLAVE_DATA                            0xA1

#define PIC_CMD_EOI                               0x20

void pic_send_eoi(uint8_t irq);

void pic_mask_irq(uint8_t irq);

void pic_unmask_irq(uint8_t irq);

void pic_init(void);

#endif /* __X86_PIC_H__ */