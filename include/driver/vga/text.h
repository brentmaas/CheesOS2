#ifndef _CHEESOS2_DRIVER_VGA_TEXT_H
#define _CHEESOS2_DRIVER_VGA_TEXT_H

#include <stdint.h>
#include <stdbool.h>

// Text mode assumes the VGA is in odd/even addressing, and
// plane 0 and 1 are enabled in the mask register.
// The text mode memory is assumed to be mapped to 0xB8000.
// The GRC registers (set/reset, enable set/reset, data rotate, operation)
// are assumed to be 0.

enum vga_attr {
    VGA_ATTR_BLACK = 0,
    VGA_ATTR_BLUE = 1,
    VGA_ATTR_GREEN = 2,
    VGA_ATTR_CYAN = 3,
    VGA_ATTR_RED = 4,
    VGA_ATTR_MAGENTA = 5,
    VGA_ATTR_YELLOW = 6,
    VGA_ATTR_GRAY = 7,

    VGA_ATTR_LIGHT = 8,
    VGA_ATTR_BLINK = 8,
    VGA_ATTR_CHARSET_B = 8,

    VGA_ATTR_WHITE = VGA_ATTR_GRAY | VGA_ATTR_LIGHT,
};

static uint8_t vga_make_attr(enum vga_attr fg, enum vga_attr bg) {
    return fg | bg << 4;
}

#define VGA_FONT_ROWS (32)
#define VGA_FONT_GLYPHS (256)

typedef uint8_t vga_glyph[VGA_FONT_ROWS];
typedef vga_glyph vga_font[VGA_FONT_GLYPHS];

struct vga_font_options {
    uint8_t text_height;
    struct {
        uint8_t start;
        uint8_t end;
    } cursor;
    bool enable_blink;
    bool enable_line_graphics;
};

void vga_clear_text(uint8_t clearchar, uint8_t clearattr);

void vga_write_str(uint8_t col, uint8_t row, const char* ptr, uint8_t attr);
void vga_write_char(uint8_t col, uint8_t row, uint8_t value, uint8_t attr);

void vga_scroll_text(uint8_t clearchar, uint8_t clearattr, int rows);

void vga_set_cursor(uint8_t col, uint8_t row);
void vga_enable_cursor(bool enabled);

void vga_upload_font(uint8_t charset, const vga_font font);
void vga_set_charsets(uint8_t charset_a, uint8_t charset_b);
void vga_set_fontopts(const struct vga_font_options* fopts);

uint8_t vga_get_height_chars(void);

#endif
