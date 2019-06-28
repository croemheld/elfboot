#include <elfboot/core.h>
#include <elfboot/io.h>

#include <asm/pic.h>

void pic_send_eoi(uint8_t irq)
{
	if (irq >= 8)
		outb(PIC_SLAVE_CMD, PIC_CMD_EOI);

	outb(PIC_MASTER_CMD, PIC_CMD_EOI);
}

void pic_init(void)
{
	/* Cascading mode */
	outb(PIC_MASTER_CMD,  0x11);
	outb(PIC_SLAVE_CMD,   0x11);

	/* Vector offsets */
	outb(PIC_MASTER_DATA, 0x20);
	outb(PIC_SLAVE_DATA,  0x28);

	/* Master-Slave link */
	outb(PIC_MASTER_DATA, 0x04);
	outb(PIC_SLAVE_DATA,  0x02);

	/* 8086 mode */
	outb(PIC_MASTER_DATA, 0x01);
	outb(PIC_SLAVE_DATA,  0x01);

	/* Reset masks */
	outb(PIC_MASTER_DATA, 0x00);
	outb(PIC_SLAVE_DATA,  0x00);
}