#ifndef __ELFBOOT_INPUT_H__
#define __ELFBOOT_INPUT_H__

#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/interrupts.h>

/*
 * Keyboard input
 */

#define KEY_ARROW_UP	0xA1
#define KEY_ARROW_LEFT	0xA2
#define KEY_ARROW_RIGHT	0xA3
#define KEY_ARROW_DOWN	0xA4

#define KEY_BACKSPACE	'\b'
#define KEY_ENTER		'\n'

#define KEYBOARD_BUFFER_SIZE	127

extern char *keybuffer;

/*
 * Keyboard functions
 */

int input_copy_to_buffer(char c);

/*
 * Formatting input
 */

unsigned char bgetch_noblock(void);

unsigned char bgetch(void);

int bgets(char *buffer, int length);

#endif /* __ELFBOOT_INPUT_H__ */