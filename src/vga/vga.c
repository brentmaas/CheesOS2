#include "vga/vga.h"

#include <string.h>

#define TAB_SIZE (4u)

static size_t vga_row, vga_column;
static uint8_t vga_current_color;
static volatile uint16_t* const vga_buffer = (uint16_t*)0xB8000;

static uint16_t vga_make_color(enum vga_color_type foreground, enum vga_color_type background) {
    return foreground | (background << 4);
}

static uint16_t vga_make_char(char c, uint8_t color) {
    return c | color << 8;
}

static void vga_write_char(char c, uint8_t color) {
    if(c == '\t') {
        size_t num_vals = vga_column % TAB_SIZE == 0 ? TAB_SIZE : vga_column % TAB_SIZE;
        for(size_t i = 0; i < num_vals; ++i)
            vga_write_char(' ', color);
        return;
    }
    if(c == '\n' || vga_column >= VGA_WIDTH) {
        vga_column = 0;

        if(vga_row == VGA_HEIGHT - 1) {
            vga_scroll(1);
        }else{
            ++vga_row;
        }
    }
    if(c != '\n') {
        vga_buffer[vga_row * VGA_WIDTH + vga_column] = vga_make_char(c, color);
        ++vga_column;
    }

}

void vga_init(void) {
    vga_row = 0;
    vga_column = 0;

    vga_current_color = vga_make_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);

    for(size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; ++i) {
        vga_buffer[i] = vga_make_char(' ', vga_current_color);
    }
}

void vga_color(enum vga_color_type foreground, enum vga_color_type background) {
    vga_current_color = vga_make_color(foreground, background);
}

void vga_putchar(char c) {
    vga_write_char(c, vga_current_color);
}

void vga_write(const char* str, size_t size) {
    for(size_t i = 0; i < size; ++i) {
        vga_putchar(str[i]);
    }
}

void vga_print(const char* str) {
    vga_write(str, strlen(str));
}

void vga_scroll(size_t rows) {
    if(rows > VGA_WIDTH)
        rows = VGA_WIDTH;

    size_t scroll_rows = VGA_HEIGHT - rows;
    size_t scroll_entries = scroll_rows * VGA_WIDTH;

    size_t i;
    for(i = 0; i < scroll_entries; ++i) {
        vga_buffer[i] = vga_buffer[i+rows*VGA_WIDTH];
    }
    for(; i < VGA_WIDTH * VGA_HEIGHT; ++i) {
        vga_buffer[i] = vga_make_char(' ', vga_current_color);
    }
}