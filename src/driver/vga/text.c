#include "driver/vga/text.h"
#include "driver/vga/io.h"
#include "driver/vga/registers.h"
#include "driver/vga/util.h"
#include "driver/vga/videomode.h"

#include "core/io.h"
#include "debug/assert.h"

#include <stddef.h>

static volatile uint8_t* TEXT_VRAM = (volatile uint8_t*) 0xB8000;
static const enum vga_plane_bits CHAR_PLANE = VGA_PLANE_0_BIT;
static const enum vga_plane_bits ATTR_PLANE = VGA_PLANE_1_BIT;
static const enum vga_plane_bits FONT_PLANE = VGA_PLANE_2_BIT;
static const enum vga_plane_bits TEXT_PLANES = CHAR_PLANE | ATTR_PLANE;

static struct {
    uint8_t height_chars;
} VGA_TEXT_SETTINGS = {};

// Offset in bytes in plane 2 of the charset
static uint16_t charset_offset(uint8_t charset) {
    return charset * 0x4000;
}

static uint16_t pos_to_offset(uint8_t col, uint8_t row) {
    return ((uint16_t) row * vga_get_width_chars() + col) * 2;
}

static uint16_t max_offset() {
    return vga_get_width_chars() * vga_get_height_chars() * 2;
}

static bool pos_in_range(uint8_t col, uint8_t row) {
    return col < vga_get_width_chars() && row < vga_get_height_chars();
}

void vga_clear_text(uint8_t clearchar, uint8_t clearattr) {
    uint16_t buf_max = max_offset();

    for (size_t i = 0; i < buf_max;) {
        TEXT_VRAM[i++] = clearchar;
        TEXT_VRAM[i++] = clearattr;
    }
}

void vga_write_str(uint8_t col, uint8_t row, const char* ptr, uint8_t attr) {
    assert(pos_in_range(col, row));

    uint16_t offset = pos_to_offset(col, row);
    uint16_t buf_max = max_offset();
    while (offset < buf_max && *ptr) {
        TEXT_VRAM[offset++] = *ptr++;
        TEXT_VRAM[offset++] = attr;
    }
}

void vga_write_char(uint8_t col, uint8_t row, uint8_t value, uint8_t attr) {
    assert(pos_in_range(col, row));

    uint16_t offset = pos_to_offset(col, row);
    TEXT_VRAM[offset] = value;
    TEXT_VRAM[offset + 1] = attr;
}

void vga_scroll_text(uint8_t clearchar, uint8_t clearattr, int rows) {
    uint8_t w = vga_get_width_chars();
    uint8_t h = vga_get_height_chars();
    uint16_t buf_max = max_offset();

    if (rows > 0) {
        if (rows > h) {
            rows = h;
        }

        size_t offset = w * 2 * rows;
        size_t i = 0;
        while (i + offset < buf_max) {
            TEXT_VRAM[i] = TEXT_VRAM[i + offset];
            ++i;
        }

        while (i < buf_max) {
            TEXT_VRAM[i++] = clearchar;
            TEXT_VRAM[i++] = clearattr;
        }
    } else if (rows < 0) {
        rows = -rows > h ? h : -rows;

        size_t offset = w * 2 * rows;
        size_t i = buf_max;
        while (i - offset > 0) {
            --i;
            TEXT_VRAM[i] = TEXT_VRAM[i - offset];
        }

        while (i > 0) {
            TEXT_VRAM[--i] = clearattr;
            TEXT_VRAM[--i] = clearchar;
        }
    }
}

void vga_set_cursor(uint8_t col, uint8_t row) {
    assert(pos_in_range(col, row));

    uint16_t offset = (uint16_t) row * (uint16_t) vga_get_width_chars() + col;

    io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_CURSOR_LOCATION_HIGH);
    VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, (uint8_t) (offset >> 8));
    io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_CURSOR_LOCATION_LOW);
    VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, (uint8_t) offset);
}

void vga_enable_cursor(bool enabled) {
    io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_CURSOR_START);
    struct vga_crtc_cursor_start cursor_start;
    VGA_READ(VGA_PORT_CRTC_COLOR_DATA, &cursor_start);
    cursor_start.disable_cursor = !enabled;
    VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, cursor_start);
}

void vga_upload_font(uint8_t charset, const vga_font font) {
    const enum vga_memory_map map = VGA_MEMORY_MAP_A0000_64K;
    volatile uint8_t* vram = vga_memory_map_ptr(map);

    bool orig_odd_even = vga_enable_odd_even(false);
    enum vga_plane_bits orig_mask = vga_mask_planes(FONT_PLANE);
    enum vga_memory_map orig_map = vga_map_memory(map);

    for (size_t i = 0; i < VGA_FONT_GLYPHS; ++i) {
        for (size_t j = 0; j < VGA_FONT_ROWS; ++j) {
            size_t k = i * VGA_FONT_ROWS + j + charset_offset(charset);
            vram[k] = font[i][j];
        }
    }

    vga_map_memory(orig_map);
    vga_mask_planes(orig_mask);
    vga_enable_odd_even(orig_odd_even);
}

void vga_set_charsets(uint8_t charset_a, uint8_t charset_b) {
    io_out8(VGA_PORT_SEQ_ADDR, VGA_IND_SEQ_CHARACTER_MAP_SELECT);
    VGA_WRITE(VGA_IND_SEQ_CHARACTER_MAP_SELECT, ((struct vga_seq_character_map_select) {
        .csbl = (charset_b >> 1) & 3,
        .csal = (charset_a >> 1) & 3,
        .csbh = charset_b & 1,
        .csah = charset_a & 1
    }));
}

void vga_set_fontopts(const struct vga_font_options* fopts) {
    vga_prepare_atc(VGA_IND_ATC_MODE_CONTROL, true);
    struct vga_atc_mode_control atc_mode;
    VGA_READ(VGA_PORT_ATC, &atc_mode);
    atc_mode.enable_blink = fopts->enable_blink;
    atc_mode.enable_line_graphics = fopts->enable_line_graphics;
    VGA_WRITE(VGA_PORT_ATC, atc_mode);

    io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_MAXIMUM_SCAN_LINE);
    struct vga_crtc_maximum_scan_line crtc_max_scanline;
    VGA_READ(VGA_PORT_CRTC_COLOR_DATA, &crtc_max_scanline);
    crtc_max_scanline.maximum_scan_line = fopts->text_height - 1;
    VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, crtc_max_scanline);

    io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_CURSOR_START);
    struct vga_crtc_cursor_start cursor_start;
    VGA_READ(VGA_PORT_CRTC_COLOR_DATA, &cursor_start);
    cursor_start.cursor_scan_line_start = fopts->cursor.start;
    VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, cursor_start);

    io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_CURSOR_END);
    VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, ((struct vga_crtc_cursor_end) {
        .cursor_scan_line_end = fopts->cursor.end - 1,
        .cursor_skew = 0
    }));

    VGA_TEXT_SETTINGS.height_chars = vga_get_height_pixels() / fopts->text_height;
}

uint8_t vga_get_height_chars(void) {
    return VGA_TEXT_SETTINGS.height_chars;
}
