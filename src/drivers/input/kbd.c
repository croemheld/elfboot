#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/mm.h>
#include <elfboot/io.h>
#include <elfboot/input.h>
#include <elfboot/interrupts.h>
#include <elfboot/module.h>
#include <elfboot/string.h>
#include <elfboot/printf.h>

#include <drivers/kbd.h>

static struct kbd_layout kbd_layout = {
#ifdef CONFIG_DRIVER_KBD_LAYOUT_DE
	.regular = {
/* 0x00 */	INV, INV, '1', '2',  '3', '4',  '5', '6',
/* 0x08 */	'7', '8', '9', '0',  INV, '?', '\b', TAB,
/* 0x10 */	'q', 'w', 'e', 'r',  't', 'z',  'u', 'i', 
/* 0x18 */	'o', 'p', INV, '+', '\n', INV,  'a', 's', 
/* 0x20 */	'd', 'f', 'g', 'h',  'j', 'k',  'l', INV, 
/* 0x28 */	INV, INV, INV, '#',  'y', 'x',  'c', 'v',
/* 0x30 */	'b', 'n', 'm', ',',  '.', '-',  INV, INV,
/* 0x38 */	INV, ' ', INV, INV,  INV, INV,  INV, INV,
/* 0x40 */	INV, INV, INV, INV,  INV, INV,  INV, INV,
/* 0x48 */	AUP, INV, INV, ALE,  INV, ARI,  INV, INV,
/* 0x50 */	ADO, INV, INV, INV,  INV, INV,  INV, INV,
/* 0x58 */	INV, INV, INV, INV,  INV, INV,  INV, INV
	},
	.shifted = {
/* 0x00 */	INV, INV, '!', '"',  'S', '$',  '%', '&',
/* 0x08 */	'/', '(', ')', '=',  '?', '`', '\b', TAB,
/* 0x10 */	'Q', 'W', 'E', 'R',  'T', 'Z',  'U', 'I', 
/* 0x18 */	'O', 'P', '?', '*',  '\n', INV, 'A', 'S', 
/* 0x20 */	'D', 'F', 'G', 'H',  'J', 'K',  'L', INV, 
/* 0x28 */	INV, INV, INV, '\'', 'Y', 'X',  'C', 'V',
/* 0x30 */	'B', 'N', 'M', ';',  ':', '_',  INV, INV,
/* 0x38 */	INV, ' ', INV, INV,  INV, INV,  INV, INV,
/* 0x40 */	INV, INV, INV, INV,  INV, INV,  INV, INV,
/* 0x48 */	AUP, INV, INV, ALE,  INV, ARI,  INV, INV,
/* 0x50 */	ADO, INV, INV, INV,  INV, INV,  INV, INV,
/* 0x58 */	INV, INV, INV, INV,  INV, INV,  INV, INV
	},
	.altgram = {
/* 0x00 */	INV, INV, INV, INV,  INV, INV,  INV, INV,
/* 0x08 */	'{', '[', ']', '}', '\\', INV, '\b', TAB,
/* 0x10 */	'@', INV, INV, INV,  INV, INV,  INV, INV,
/* 0x18 */	INV, INV, INV, '~', '\n', INV,  INV, INV,
/* 0x20 */	INV, INV, INV, INV,  INV, INV,  INV, INV,
/* 0x28 */	INV, INV, INV, INV,  INV, INV,  INV, INV,
/* 0x30 */	INV, INV, INV, INV,  INV, INV,  INV, INV,
/* 0x38 */	INV, ' ', INV, INV,  INV, INV,  INV, INV,
/* 0x40 */	INV, INV, INV, INV,  INV, INV,  INV, INV,
/* 0x48 */	AUP, INV, INV, ALE,  INV, ARI,  INV, INV,
/* 0x50 */	ADO, INV, INV, INV,  INV, INV,  INV, INV,
/* 0x58 */	INV, INV, INV, INV,  INV, INV,  INV, INV
	}
#endif
};

static struct kbd_status kbd_status = {
	.layout = &kbd_layout,
	.status = 0
};

/*
 * Scan codes utility functions
 */

static bool key_release(uint8_t scancode)
{
	return !!(scancode & 0x80);
}

static bool key_shifted(uint8_t scancode)
{
	return (scancode == KBD_CODE_LSHIFT) || (scancode == KBD_CODE_RSHIFT);
}

static bool key_altgram(uint8_t scancode)
{
	return (scancode == KBD_CODE_ALT);
}

static bool key_control(uint8_t scancode)
{
	return (scancode == KBD_CODE_CTRL);
}

static bool key_capslck(uint8_t scancode)
{
	return (scancode == KBD_CODE_CAPSLOCK);
}

static char key_getchar(uint8_t scancode)
{
	bool shifted, altgram;

	shifted = !!(kbd_status.status & KBD_META_SHIFT);
	if (kbd_status.status & KBD_META_CAPSLOCK)
		shifted = !shifted;

	altgram = !!(kbd_status.status & KBD_META_ALTGR);

	if (shifted & altgram)
		return INV;

	if (shifted)
		return kbd_status.layout->shifted[scancode];
	else if (altgram)
		return kbd_status.layout->altgram[scancode];
	else
		return kbd_status.layout->regular[scancode];
}

/*
 * Key handlers for pressing and releasing
 */

static void kbd_handle_release(uint8_t scancode)
{
	if (key_shifted(scancode))
		kbd_status.status &= ~KBD_META_SHIFT;
	else if (key_altgram(scancode))
		kbd_status.status &= ~KBD_META_ALTGR;
	else if (key_control(scancode))
		kbd_status.status &= ~KBD_META_CTRL;
}

static void kbd_handle_pressed(uint8_t scancode)
{
	char c;

	if (key_shifted(scancode))
		kbd_status.status |= KBD_META_SHIFT;
	else if (key_altgram(scancode))
		kbd_status.status |= KBD_META_ALTGR;
	else if (key_control(scancode))
		kbd_status.status |= KBD_META_CTRL;
	else if (key_capslck(scancode))
		kbd_status.status ^= KBD_META_CAPSLOCK;
	else {
		/*
		 * We ignore scancodes which are outside of the mapped keyboard layout
		 * (for 101-, 102- and 104 keyboards). Each keyboard layout contains a
		 * map for KBD_LAYOUT_SIZE characters (printables & non-printables).
		 */
		if (scancode >= KBD_LAYOUT_SIZE)
			return;

		c = key_getchar(scancode);
		if (!c)
			return;

		input_copy_to_buffer(c);
	}
}

static void kbd_interrupt_callback(void *info __unused)
{
	uint8_t scancode = inb(0x60);

	if (key_release(scancode))
		kbd_handle_release(scancode & ~(0x80));
	else
		kbd_handle_pressed(scancode & ~(0x80));
}

static struct interrupt_handler kbd_interrupt_handler = {
	.name = DRIVER_KBD,
	.callback = kbd_interrupt_callback
};

static int kbd_init(void)
{
	bprintln(DRIVER_KBD ": Initialize module...");

	keybuffer = bzalloc(KBD_BUFFER_SIZE + 1);
	if (!keybuffer)
		return -ENOMEM;

	/*
	 * Drain keyboard buffer before continuing. We have to verify whether 
	 * bit 0 is set as it indicates that the output buffer status is full
	 */
	while (inb(0x64) & 0x01);

	register_interrupt_handler(KBD_IRQ, &kbd_interrupt_handler);

	return 0;
}

static void kbd_exit(void)
{
	/*
	 * Not supported.
	 */

	bprintln(DRIVER_KBD ": Exit module...");

	disable_keyboard();
}

module_init(kbd_init);
module_exit(kbd_exit);