#ifndef __BOOT_BDA_H__
#define __BOOT_BDA_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include <uapi/elfboot/common.h>

#define BDA_ADDRESS                               0x400
#define BDA_MAX_SIZE                              0x100

#define EBDA_MAX_SIZE                             (128 * 1024)
#define EBDA_LOWMEM_LIMIT                         0xA0000
#define EBDA_LOWMEM_START                         (EBDA_LOWMEM_LIMIT - EBDA_MAX_SIZE)

struct bios_data_area {
	uint16_t com[4];
	uint16_t lpt[3];
	uint16_t ebda_segment;
	uint16_t equipment;
	uint8_t  post_status;
	uint16_t low_mem_size;
	uint16_t _reserved1;

	uint8_t  kbd_status1;
	uint8_t  kbd_status2;
	uint8_t  kbd_alt_keypad;
	uint16_t kbd_head;
	uint16_t kbd_tail;
	uint16_t kbd_buffer;

	uint8_t  floppy_recalibrate;
	uint8_t  floppy_motor;
	uint8_t  floppy_timeout;
	uint8_t  floppy_status;
	uint8_t  floppy_command[7];

	uint8_t  video_mode;
	uint16_t video_columns;
	uint16_t video_page_size;
	uint16_t video_page_addr;
	uint8_t  video_cursor[16];
	uint16_t video_cursor_type;
	uint8_t  video_page;
	uint16_t video_crt_addr;
	uint8_t  video_mode_select;
	uint8_t  video_cga_palette;

	uint32_t post_rm_entry;
	uint8_t  last_spurious_int;

	uint32_t timer_ticks;
	uint8_t  timer_overflow;

	uint8_t  kbd_ctrl_break;
	uint16_t post_reset_flags;

	uint8_t  disk_status;
	uint8_t  disk_count;
	uint8_t  disk_ctrl;
	uint8_t  disk_io_port;

	uint8_t  lpt_timeout[3];
	uint8_t  virtual_dma;
	uint8_t  com_timeout[4];

	uint16_t kbd_buffer_start;
	uint16_t kbd_buffer_end;

	uint8_t  video_rows;
	uint16_t bytes_per_char;
	uint8_t  video_options;
	uint8_t  video_switches;
	uint8_t  video_control;
	uint8_t  video_dcc_idx;

	uint8_t  floppy_data_rate;
	uint8_t  disk_ctrlr_status;
	uint8_t  disk_ctrlr_error;
	uint8_t  disk_complete;
	uint8_t  floppy_info;
	uint8_t  drive_state[4];
	uint8_t  floppy_track[2];

	uint8_t  kbd_mode;
	uint8_t  kbd_led_status;

	uint32_t timer2_ptr;
	uint32_t timer2_timeout;
	uint8_t  timer2_wait_active;

	uint8_t  lan_a_channel;
	uint8_t  lan_a_status[2];
	uint32_t disk_ivt;
	uint32_t video_ptr;
	uint8_t  _reserved2[8];
	uint8_t  kbd_nmi;
	uint32_t kbd_break_pending;
	uint8_t  port_60_queue;
	uint8_t  scancode;
	uint8_t  nmi_head;
	uint8_t  nmi_tail;
	uint8_t  nmi_buffer[16];
	uint8_t  _reserved3;
	uint16_t day;
	uint8_t  _reserved4[32];
	uint8_t  userspace[16];
	uint8_t  print_screen;
} __attribute__((packed));

struct bios_data_area *bios_get_bda(void);

uint32_t bios_get_ebda_addr(void);

#define EBDA_ADDRESS                              bios_get_ebda_addr()

void *bios_get_ebda_ptr(void);

#endif /* __BOOT_BDA_H__ */