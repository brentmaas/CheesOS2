#ifndef _CHEESOS2_INCLUDE_VGA_VGA_H
#define _CHEESOS2_INCLUDE_VGA_VGA_H

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#define VGA_WIDTH (80)
#define VGA_HEIGHT (25)

enum vga_color_type {
    VGA_COLOR_BLACK,
    VGA_COLOR_BLUE,
    VGA_COLOR_GREEN,
    VGA_COLOR_CYAN,
    VGA_COLOR_RED,
    VGA_COLOR_MAGENTA,
    VGA_COLOR_BROWN,
    VGA_COLOR_LIGHT_GREY,
    VGA_COLOR_DARK_GREY,
    VGA_COLOR_LIGHT_BLUE,
    VGA_COLOR_LIGHT_GREEN,
    VGA_COLOR_LIGHT_CYAN,
    VGA_COLOR_LIGHT_RED,
    VGA_COLOR_LIGHT_MAGENTA,
    VGA_COLOR_LIGHT_BROWN,
    VGA_COLOR_WHITE
};

void vga_init(void);
void vga_color(enum vga_color_type, enum vga_color_type);
void vga_putchar(char c);
void vga_write(const char*, size_t);
void vga_print(const char*);
void vga_printf(const char*, ...);
void vga_vprintf(const char*, va_list);
void vga_scroll(size_t);

#endif
