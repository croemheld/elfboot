#include <elfboot/core.h>
#include <elfboot/io.h>

#include <asm/pic.h>

static uint16_t irq_mask = 0xFFFF & ~(0x04);

void pic_send_eoi(uint8_t irq)
{
	if (irq >= 0x28)
		outb(PIC_SLAVE_CMD, PIC_CMD_EOI);
	
	outb(PIC_MASTER_CMD, PIC_CMD_EOI);
}

void pic_mask_irq(uint8_t irq)
{
	uint16_t mask = irq_mask | (1 << irq);

	/* Update mask */
	outb(PIC_MASTER_DATA, (mask >> 0) & 0xFF);
	outb(PIC_SLAVE_DATA,  (mask >> 8) & 0xFF);

	irq_mask = mask;
}

void pic_unmask_irq(uint8_t irq)
{
	uint16_t mask = irq_mask & ~(1 << irq);

	/* Update mask */
	outb(PIC_MASTER_DATA, (mask >> 0) & 0xFF);
	outb(PIC_SLAVE_DATA,  (mask >> 8) & 0xFF);

	irq_mask = mask;
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

	/* Mask interrupts */
	outb(PIC_MASTER_DATA, 0xFF);
	outb(PIC_SLAVE_DATA,  0xFF);
}