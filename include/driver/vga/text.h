#ifndef _CHEESOS2_DRIVER_VGA_TEXT_H
#define _CHEESOS2_DRIVER_VGA_TEXT_H

#include <stdint.h>
#include <stdbool.h>

// Text mode assumes the VGA is in odd/even addressing, and
// plane 0 and 1 are enabled in the mask register.
// The text mode memory is assumed to be mapped to 0xB8000.
// The GRC registers (set/reset, enable set/reset, data rotate, operation)
// are assumed to be 0.

#define VGA_FONT_ROWS 32
#define VGA_FONT_GLYPHS 256

typedef uint8_t vga_glyph[VGA_FONT_ROWS];
typedef vga_glyph vga_font[VGA_FONT_GLYPHS];

typedef struct {
    uint8_t text_height;
    struct {
        uint8_t start;
        uint8_t end;
    } cursor;
    bool enable_blink;
    bool enable_line_graphics;
} vga_font_options;

void vga_clear_text();

void vga_write_str(uint8_t col, uint8_t row, const char* ptr, uint8_t attr);
void vga_write_char(uint8_t col, uint8_t row, uint8_t value, uint8_t attr);

void vga_set_cursor(uint8_t col, uint8_t row);
void vga_enable_cursor(bool enabled);

void vga_upload_font(uint8_t charset, const vga_font* font);
void vga_set_charsets(uint8_t charset_a, uint8_t charset_b);
void vga_set_fontopts(const vga_font_options* fopts);

uint8_t vga_get_height_chars();

#endif
