#include "driver/vga/text.h"
#include "driver/vga/io.h"
#include "driver/vga/registers.h"
#include "driver/vga/util.h"
#include "core/io.h"
#include <stddef.h>

static volatile uint8_t* TEXT_VRAM = (volatile uint8_t*) 0xB8000;
static const vga_plane_bits CHAR_PLANE = VGA_PLANE_0_BIT;
static const vga_plane_bits ATTR_PLANE = VGA_PLANE_1_BIT;
static const vga_plane_bits FONT_PLANE = VGA_PLANE_2_BIT;
static const vga_plane_bits TEXT_PLANES = CHAR_PLANE | ATTR_PLANE;

// Offset in bytes in plane 2 of the charset
static uint16_t charset_offset(uint8_t charset) {
    return charset * 0x4000;
}

void vga_set_char(uint8_t col, uint8_t row, uint8_t value, uint8_t attr) {

}

void vga_upload_font(uint8_t charset, const vga_font* font) {
    const vga_memory_map map = VGA_MEMORY_MAP_A0000_64K;
    volatile uint8_t* vram = vga_memory_map_ptr(map);

    bool orig_odd_even = vga_enable_odd_even(false);
    vga_plane_bits orig_mask = vga_mask_planes(FONT_PLANE);
    vga_memory_map orig_map = vga_map_memory(map);

    for (size_t i = 0; i < VGA_FONT_GLYPHS; ++i) {
        for (size_t j = 0; j < VGA_FONT_ROWS; ++j) {
            size_t k = i * VGA_FONT_ROWS + j;
            vram[k] = (*font)[i][j];
        }
    }

    vga_map_memory(orig_map);
    vga_mask_planes(orig_mask);
    vga_enable_odd_even(orig_odd_even);
}

void vga_set_charsets(uint8_t charset_a, uint8_t charset_b) {
    io_out8(VGA_PORT_SEQ_ADDR, VGA_IND_SEQ_CHARACTER_MAP_SELECT);
    VGA_WRITE(VGA_IND_SEQ_CHARACTER_MAP_SELECT, ((vga_seq_character_map_select) {
        .csbl = (charset_b >> 1) & 3,
        .csal = (charset_a >> 1) & 3,
        .csbh = charset_b & 1,
        .csah = charset_a & 1
    }));
}

void vga_set_fontopts(const vga_font_options* fopts) {
    vga_prepare_atc(VGA_IND_ATC_MODE_CONTROL);
    vga_atc_mode_control atc_mode;
    VGA_READ(VGA_PORT_ATC, &atc_mode);
    atc_mode.enable_blink = fopts->enable_blink;
    atc_mode.enable_line_graphics = fopts->enable_line_graphics;
    VGA_WRITE(VGA_PORT_ATC, atc_mode);

    io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_MAXIMUM_SCAN_LINE);
    vga_crtc_maximum_scan_line crtc_max_scanline;
    VGA_READ(VGA_PORT_CRTC_COLOR_DATA, &crtc_max_scanline);
    crtc_max_scanline.maximum_scan_line = fopts->text_height;
    VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, crtc_max_scanline);

    io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_CURSOR_START);
    vga_crtc_cursor_start cursor_start;
    VGA_READ(VGA_PORT_CRTC_COLOR_DATA, &cursor_start);
    cursor_start.cursor_scan_line_start = fopts->cursor.start;
    VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, cursor_start);

    io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_CURSOR_END);
    VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, ((vga_crtc_cursor_end) {
        .cursor_scan_line_end = fopts->cursor.end,
        .cursor_skew = 0
    }));
}

void vga_enable_cursor(bool enabled) {
    io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_CURSOR_START);
    vga_crtc_cursor_start cursor_start;
    VGA_READ(VGA_PORT_CRTC_COLOR_DATA, &cursor_start);
    cursor_start.disable_cursor = !enabled;
    VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, cursor_start);
}
