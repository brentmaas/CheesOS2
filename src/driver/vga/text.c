#include "driver/vga/text.h"
#include "driver/vga/io.h"
#include "driver/vga/registers.h"
#include "core/io.h"

static volatile uint8_t* TEXT_VRAM = (volatile uint8_t*) 0xB8000;

// Offset in bytes in plane 2 of the charset
static uint16_t charset_offset(uint8_t charset) {
    return charset * 0x4000;
}

void vga_set_char(uint8_t col, uint8_t row, uint8_t value, uint8_t attr) {

}

void vga_upload_font(uint8_t charset, const vga_glyph* font) {

}

void vga_set_charsets(uint8_t charset_a, uint8_t charset_b) {
    io_out8(VGA_PORT_SEQ_ADDR, VGA_IND_SEQ_CHARACTER_MAP_SELECT);
    VGA_WRITE(VGA_IND_SEQ_CHARACTER_MAP_SELECT, ((vga_seq_character_map_select) {
        .csbl = charset_b & 3,
        .csal = charset_a & 3,
        .csbh = (charset_b >> 2) & 1,
        .csah = (charset_a >> 2) & 1
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
