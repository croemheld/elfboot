#ifndef __DRIVER_KBD_H__
#define __DRIVER_KBD_H__

#include <elfboot/core.h>
#include <elfboot/input.h>
#include <elfboot/interrupts.h>

#define DRIVER_KBD	"KBD"

#define KBD_IRQ		33

#define KBD_META_SHIFT			0x01
#define KBD_META_CTRL			0x02
#define KBD_META_ALT			0x04
#define KBD_META_CAPSLOCK		0x08
#define KBD_META_ALTGR			0x40
#define KBD_META_ESCAPED		0x80

#define KBD_CODE_LSHIFT			0x2A
#define KBD_CODE_RSHIFT			0x36
#define KBD_CODE_CTRL			0x1D
#define KBD_CODE_CAPSLOCK		0x3A
#define KBD_CODE_BACKSPACE		0x0E
#define KBD_CODE_ALT			0x38
#define KBD_CODE_ENTER			0x1C
#define KBD_CODE_ESC			0x01

#define INV	0x00
#define TAB	0x09

#define AUP	KEY_ARROW_UP
#define ALE	KEY_ARROW_LEFT
#define ADO	KEY_ARROW_RIGHT
#define ARI	KEY_ARROW_DOWN

#define KBD_BUFFER_SIZE	KEYBOARD_BUFFER_SIZE
#define KBD_LAYOUT_SIZE	 96

struct kbd_layout {
	const char regular[KBD_LAYOUT_SIZE];
	const char shifted[KBD_LAYOUT_SIZE];
	const char altgram[KBD_LAYOUT_SIZE];
};

struct kbd_status {
	struct kbd_layout *layout;
	uint32_t status;
};

#endif /* __DRIVER_KBD_H__ */