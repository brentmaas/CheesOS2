#ifndef _CHEESOS2_DRIVER_VGA_REGISTERS_H
#define _CHEESOS2_DRIVER_VGA_REGISTERS_H

#include <stdint.h>

// Graphics registers
typedef enum {
    VGA_IND_GRC_SET_RESET = 0x00,
    VGA_IND_GRC_ENABLE_SET_RESET = 0x01,
    VGA_IND_GRC_COLOR_COMPARE = 0x02,
    VGA_IND_GRC_DATA_ROTATE = 0x03,
    VGA_IND_GRC_READ_MAP = 0x04,
    VGA_IND_GRC_MODE = 0x05,
    VGA_IND_GRC_MISC = 0x06,
    VGA_IND_GRC_COLOR_DONT_CARE = 0x07,
    VGA_IND_GRC_BIT_MASK = 0x08
} vga_grc_index;
#define VGA_NUM_GRC_INDICES (0x09)

typedef enum {
    VGA_PLANE_0_BIT = 1 << 0,
    VGA_PLANE_1_BIT = 1 << 1,
    VGA_PLANE_2_BIT = 1 << 2,
    VGA_PLANE_3_BIT = 1 << 3,
} vga_plane_bits;

#define VGA_PLANE_ALL (VGA_PLANE_0_BIT | VGA_PLANE_1_BIT | VGA_PLANE_2_BIT | VGA_PLANE_3_BIT)
#define VGA_PLANE_NONE 0

typedef enum {
    VGA_OP_NOP = 0x00,
    VGA_OP_AND = 0x01,
    VGA_OP_OR = 0x02,
    VGA_OP_XOR = 0x03
} vga_operator;

typedef struct __attribute__((packed)) {
    vga_plane_bits planes : 4;
    uint8_t : 4;
} vga_grc_set_reset;

typedef struct __attribute__((packed)) {
    vga_plane_bits planes : 4;
    uint8_t : 4;
} vga_grc_enable_set_reset;

typedef struct __attribute__((packed)) {
    vga_plane_bits planes : 4;
    uint8_t : 4;
} vga_grc_color_compare;

typedef struct __attribute__((packed)) {
    uint8_t rotate_count : 3;
    vga_operator operation : 2;
    uint8_t : 3;
} vga_grc_data_rotate;

typedef struct __attribute__((packed)) {
    uint8_t plane : 2;
    uint8_t : 6;
} vga_grc_read_map;

typedef struct __attribute__((packed)) {
    uint8_t write_mode : 2;
    uint8_t : 1;
    uint8_t read_mode : 1;
    uint8_t enable_host_odd_even : 1;
    uint8_t enable_shift_interleave : 1;
    uint8_t enable_shift256 : 1;
    uint8_t : 1;
} vga_grc_mode;

typedef enum {
    VGA_MEMORY_MAP_A0000_128K = 0x00,
    VGA_MEMORY_MAP_A0000_64K = 0x01,
    VGA_MEMORY_MAP_B0000_32K = 0x02,
    VGA_MEMORY_MAP_B8000_32K = 0x03
} vga_memory_map;

static volatile uint8_t* vga_memory_map_ptr(vga_memory_map map) {
    switch (map) {
        case VGA_MEMORY_MAP_A0000_128K:
        case VGA_MEMORY_MAP_A0000_64K:
            return (volatile uint8_t*) 0xA0000;
        case VGA_MEMORY_MAP_B0000_32K:
            return (volatile uint8_t*) 0xB0000;
        case VGA_MEMORY_MAP_B8000_32K:
            return (volatile uint8_t*) 0xB8000;
    }

    __builtin_unreachable();
}

typedef struct __attribute__((packed)) {
    uint8_t enable_graphics_mode : 1;
    uint8_t enable_chain_odd_even : 1;
    vga_memory_map memory_map : 2;
    uint8_t : 4;
} vga_grc_misc;

// Sequencer registers
typedef enum {
    VGA_IND_SEQ_RESET = 0x00,
    VGA_IND_SEQ_CLOCKING_MODE = 0x01,
    VGA_IND_SEQ_MAP_MASK = 0x02,
    VGA_IND_SEQ_CHARACTER_MAP_SELECT = 0x03,
    VGA_IND_SEQ_MEMORY_MODE = 0x04
} vga_seq_index;
#define VGA_NUM_SEQ_INDICES (0x05)

typedef struct __attribute__((packed)) {
    uint8_t synchronous_reset : 1;
    uint8_t asynchronous_reset : 1;
    uint8_t : 6;
} vga_seq_reset;

typedef enum {
    VGA_DOT_MODE_9_DPC = 0,
    VGA_DOT_MODE_8_DPC = 1,
} vga_dot_mode;

typedef struct __attribute__((packed)) {
    vga_dot_mode dot_mode : 1;
    uint8_t _ : 1;
    uint8_t shift_load_rate : 1;
    uint8_t enable_half_dot_clock : 1;
    uint8_t enable_shift_4 : 1;
    uint8_t disable_display : 1;
    uint8_t : 2;
} vga_seq_clocking_mode;

typedef struct __attribute__((packed)) {
    vga_plane_bits planes : 4;
    uint8_t : 4;
} vga_seq_map_mask;

typedef struct __attribute__((packed)) {
    uint8_t csbl : 2;
    uint8_t csal : 2;
    uint8_t csbh : 1;
    uint8_t csah : 1;
    uint8_t : 2;
} vga_seq_character_map_select;

typedef struct __attribute__((packed)) {
    uint8_t : 1;
    uint8_t extended_memory : 1;
    uint8_t disable_odd_even : 1;
    uint8_t enable_chain_4 : 1;
    uint8_t : 4;
} vga_seq_memory_mode;

// Attribute controller registers
// 0x00 - 0x0F are palette index registers
typedef enum {
    VGA_IND_ATC_PALETTE_0 = 0x00,
    VGA_IND_ATC_MODE_CONTROL = 0x10,
    VGA_IND_ATC_OVERSCAN_COLOR = 0x11,
    VGA_IND_ATC_COLOR_PLANE_ENABLE = 0x12,
    VGA_IND_ATC_HORIZ_PIXEL_PANNING = 0x13,
    VGA_IND_ATC_COLOR_SELECT = 0x14,
} vga_atc_index;
#define VGA_NUM_ATC_INDICES (0x15)

typedef struct __attribute__((packed)) {
    uint8_t attribute_address : 5;
    uint8_t lock_palette : 1;
    uint8_t : 2;
} vga_atc_address;

typedef struct __attribute__((packed)) {
    uint8_t enable_graphics : 1;
    uint8_t enable_monochrome_emulation : 1;
    uint8_t enable_line_graphics : 1;
    uint8_t enable_blink : 1;
    uint8_t : 1;
    uint8_t enable_bottom_pixel_pan : 1;
    uint8_t enable_256_color : 1;
    uint8_t enable_p45_color_select : 1;
} vga_atc_mode_control;

typedef struct __attribute__((packed)) {
    vga_plane_bits planes : 4;
    uint8_t : 4;
} vga_atc_color_plane_enable;

typedef struct __attribute__((packed)) {
    uint8_t pixel_shift : 4;
    uint8_t : 4;
} vga_atc_horiz_pixel_panning;

typedef struct __attribute__((packed)) {
    uint8_t color_select_5_4 : 2;
    uint8_t color_select_7_6 : 2;
    uint8_t : 4;
} vga_atc_color_select;

// CRT controller registers
typedef enum {
    VGA_IND_CRTC_HORIZ_TOTAL = 0x00,
    VGA_IND_CRTC_HORIZ_DISPLAY_END = 0x01,
    VGA_IND_CRTC_HORIZ_BLANKING_START = 0x02,
    VGA_IND_CRTC_HORIZ_BLANKING_END = 0x03,
    VGA_IND_CRTC_HORIZ_RETRACE_START = 0x04,
    VGA_IND_CRTC_HORIZ_RETRACE_END = 0x05,
    VGA_IND_CRTC_VERT_TOTAL = 0x06,
    VGA_IND_CRTC_OVERFLOW = 0x07,
    VGA_IND_CRTC_PRESET_ROW_SCAN = 0x08,
    VGA_IND_CRTC_MAXIMUM_SCAN_LINE = 0x09,
    VGA_IND_CRTC_CURSOR_START = 0x0A,
    VGA_IND_CRTC_CURSOR_END = 0x0B,
    VGA_IND_CRTC_START_ADDRESS_HIGH = 0x0C,
    VGA_IND_CRTC_START_ADDRESS_LOW = 0x0D,
    VGA_IND_CRTC_CURSOR_LOCATION_HIGH = 0x0E,
    VGA_IND_CRTC_CURSOR_LOCATION_LOW = 0x0F,
    VGA_IND_CRTC_VERT_RETRACE_START = 0x10,
    VGA_IND_CRTC_VERT_RETRACE_END = 0x11,
    VGA_IND_CRTC_VERT_DISPLAY_END = 0x12,
    VGA_IND_CRTC_OFFSET = 0x13,
    VGA_IND_CRTC_UNDERLINE_LOCATION = 0x14,
    VGA_IND_CRTC_VERT_BLANKING_START = 0x15,
    VGA_IND_CRTC_VERT_BLANKING_END = 0x16,
    VGA_IND_CRTC_MODE = 0x17,
    VGA_IND_CRTC_LINE_COMPARE = 0x18
} vga_crtc_index;
#define VGA_NUM_CRTC_INDICES (0x19)

typedef struct __attribute__((packed)) {
    uint8_t horiz_blanking_end_low : 5;
    uint8_t enable_display_skew : 2;
    uint8_t enable_vert_retrace_access : 1;
} vga_crtc_horiz_blanking_end;

typedef struct __attribute__((packed)) {
    uint8_t horiz_retrace_end : 5;
    uint8_t horiz_retrace_skew : 2;
    uint8_t horiz_blanking_end_high : 1;
} vga_crtc_horiz_retrace_end;

typedef struct __attribute__((packed)) {
    uint8_t vert_total_8 : 1;
    uint8_t vert_display_end_8 : 1;
    uint8_t vert_retrace_start_8 : 1;
    uint8_t vert_blanking_start_8 : 1;
    uint8_t line_compare_8 : 1;
    uint8_t vert_total_9 : 1;
    uint8_t vert_display_end_9 : 1;
    uint8_t vert_retrace_start_9 : 1;
} vga_crtc_overflow;

typedef struct __attribute__((packed)) {
    uint8_t preset_row_scan : 5;
    uint8_t byte_panning : 2;
    uint8_t : 1;
} vga_crtc_preset_row_scan;

typedef struct __attribute__((packed)) {
    uint8_t maximum_scan_line : 5;
    uint8_t vert_blanking_start_9 : 1;
    uint8_t line_compare_9 : 1;
    uint8_t enable_double_scan : 1;
} vga_crtc_maximum_scan_line;

typedef struct __attribute__((packed)) {
    uint8_t cursor_scan_line_start : 5;
    uint8_t disable_cursor : 1;
    uint8_t : 2;
} vga_crtc_cursor_start;

typedef struct __attribute__((packed)) {
    uint8_t cursor_scan_line_end : 5;
    uint8_t cursor_skew : 2;
    uint8_t : 1;
} vga_crtc_cursor_end;

typedef struct __attribute__((packed)) {
    uint8_t vert_retrace_end : 4;
    uint8_t : 2;
    uint8_t bandwidth : 1;
    uint8_t protect : 1;
} vga_crtc_vert_retrace_end;

typedef struct __attribute__((packed)) {
    uint8_t underline_location : 5;
    uint8_t enable_quarter_memory_clock : 1;
    uint8_t enable_dword_addressing : 1;
    uint8_t : 1;
} vga_crtc_underline_location;

typedef struct __attribute__((packed)) {
    uint8_t map13 : 1;
    uint8_t map14 : 1;
    uint8_t enable_half_scanline_clock : 1;
    uint8_t enable_half_memory_clock : 1;
    uint8_t : 1;
    uint8_t address_wrap_select : 1;
    uint8_t disable_word_addressing : 1;
    uint8_t enable_sync : 1;
} vga_crtc_mode;

// Miscellaneous registers
typedef enum {
    VGA_EMULATION_MODE_MONOCHROME = 0x00,
    VGA_EMULATION_MODE_COLOR = 0x01
} vga_emulation_mode;

typedef enum {
    VGA_CLOCK_SPEED_25MHZ = 0x00,
    VGA_CLOCK_SPEED_28MHZ = 0x01,
    // 0x02 and 0x03 are implementation-defined
} vga_clock_speed;

typedef enum {
    VGA_POLARITY_POSITIVE = 0,
    VGA_POLARITY_NEGATIVE = 1
} vga_polarity;

typedef struct __attribute__((packed)) {
    vga_emulation_mode io_emulation_mode : 1;
    uint8_t enable_vram_access : 1;
    vga_clock_speed clock_select : 2;
    uint8_t : 1;
    uint8_t odd_even_select_high_page : 1;
    vga_polarity hsync_polarity : 1;
    vga_polarity vsync_polarity : 1;
} vga_misc;

#endif