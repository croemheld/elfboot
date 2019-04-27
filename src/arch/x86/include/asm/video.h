#ifndef __BOOT_VIDEO_H__
#define __BOOT_VIDEO_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

struct timing_description {
	uint8_t  horizontal_frequency;
	uint8_t  vertical_frequency;
	uint8_t  horizontal_active_time;
	uint8_t  horizontal_blanking_time;
	uint8_t  horizontal_active_blanking_ratio;
	uint8_t  vertical_active_time;
	uint8_t  vertical_blanking_time;
	uint8_t  vertical_active_blanking_ratio;
	uint8_t  horizontal_sync_offset;
	uint8_t  horizontal_sync_pulse_width;
	uint8_t  vertical_sync_offset;
	uint8_t  vertical_sync_pulse_width;
	uint8_t  horizontal_image_size;
	uint8_t  vertical_image_size;
	uint8_t  image_size_ratio;
	uint8_t  horizontal_border;
	uint8_t  vertical_border;
	uint8_t  display_type;
} __attribute__((packed));

struct monitor {
	uint16_t width;
	uint16_t height;
} __attribute__((packed));

struct edid_data {
	uint64_t padding;
	uint16_t manufacture_id;
	uint16_t edid_id_code;
	uint32_t serial_number;
	uint8_t  manufacture_week;
	uint8_t  manufacture_year;
	uint8_t  edid_version;
	uint8_t  edid_revision;
	uint8_t  video_input_type;
	uint8_t  max_horizontal_size;
	uint8_t  max_vertical_size;
	uint8_t  gamma_factir;
	uint8_t  dpms_flags;
	uint8_t  chroma_info[10];
	uint8_t  established_timings1;
	uint8_t  established_timings2;
	uint8_t  manufacture_reserved_timings;
	uint16_t std_timing_id[8];
	struct timing_description timing_description1;
	struct timing_description timing_description2;
	struct timing_description timing_description3;
	struct timing_description timing_description4;
	uint8_t  unused;
	uint8_t  checksum;
} __attribute__((packed));

struct vesa_info {
	char signature[4];
	uint16_t version;
	uint32_t oem;
	uint32_t capabilities;
	uint32_t video_modes;
	uint32_t video_memory;
	uint16_t software_revision;
	uint32_t vendor;
	uint32_t product_name;
	uint32_t product_revision;
	uint8_t  _reserved[222];
	uint8_t  oem_data[256];
} __attribute__((packed));

struct vesa_mode {
	uint16_t attributes;
	uint8_t  window_a;
	uint8_t  window_b;
	uint16_t granularity;
	uint16_t window_size;
	uint16_t segment_a;
	uint16_t segment_b;
	uint32_t window_fptr;
	uint16_t pitch;
	uint16_t width;
	uint16_t height;
	uint8_t  w_char;
	uint8_t  y_char;
	uint8_t  planes;
	uint8_t  bpp;
	uint8_t  banks;
	uint8_t  memory_model;
	uint8_t  bank_size;
	uint8_t  image_pages;
	uint8_t  _reserved1;
	uint8_t  red_mask;
	uint8_t  red_position;
	uint8_t  green_mask;
	uint8_t  green_position;
	uint8_t  blue_mask;
	uint8_t  blue_position;
	uint8_t  reserved_mask;
	uint8_t  reserved_position;
	uint8_t  direct_color_attributes;
	uint32_t framebuffer;
	uint32_t off_screen_memory_offset;
	uint16_t off_screen_memory_size;
	uint8_t  _reserved2[206];
} __attribute__((packed));

#endif /* __BOOT_VIDEO_H__ */