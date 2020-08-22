#include "debug/console/console.h"

#include "driver/vga/videomode.h"
#include "driver/vga/palette.h"
#include "driver/vga/text.h"
#include "res/fonts.h"

#include "utility/cprintf.h"

#include <stddef.h>

#define TAB_SIZE (4u)

const struct vga_dac_color CONSOLE_COLORS[] = {
    [VGA_ATTR_BLACK] = {0, 0, 0},
    [VGA_ATTR_BLUE] = {0, 0, 32},
    [VGA_ATTR_GREEN] = {0, 32, 0},
    [VGA_ATTR_CYAN] = {0, 32, 32},
    [VGA_ATTR_RED] = {32, 0, 0},
    [VGA_ATTR_MAGENTA] = {32, 0, 32},
    [VGA_ATTR_YELLOW] = {32, 32, 0},
    [VGA_ATTR_GRAY] = {48, 48, 48},

    [VGA_ATTR_LIGHT | VGA_ATTR_BLACK] = {32, 32, 32},
    [VGA_ATTR_LIGHT | VGA_ATTR_BLUE] = {0, 0, 63},
    [VGA_ATTR_LIGHT | VGA_ATTR_GREEN] = {0, 63, 0},
    [VGA_ATTR_LIGHT | VGA_ATTR_CYAN] = {0, 63, 63},
    [VGA_ATTR_LIGHT | VGA_ATTR_RED] = {63, 0, 0},
    [VGA_ATTR_LIGHT | VGA_ATTR_MAGENTA] = {63, 0, 63},
    [VGA_ATTR_LIGHT | VGA_ATTR_YELLOW] = {63, 63, 0},
    [VGA_ATTR_WHITE] = {63, 63, 63}
};

const enum vga_attr LOG_LEVEL_COLORS[] = {
    [LOG_LEVEL_DEBUG] = VGA_ATTR_GREEN,
    [LOG_LEVEL_INFO] = VGA_ATTR_LIGHT | VGA_ATTR_BLUE,
    [LOG_LEVEL_WARN] = VGA_ATTR_LIGHT | VGA_ATTR_YELLOW,
    [LOG_LEVEL_ERROR] = VGA_ATTR_LIGHT | VGA_ATTR_RED
};

const struct vga_font_options CONSOLE_FOPTS = {
    .text_height = 16,
    .cursor = {
        .start = 14,
        .end = 16
    },
    .enable_blink = false,
    .enable_line_graphics = false
};

static struct {
    uint8_t col;
    uint8_t row;
    uint8_t attr;
} CONSOLE_STATE = {};

void console_init(void) {
    vga_set_videomode(&VGA_VIDEOMODE_640x480, VGA_MODE_TEXT);
    vga_dac_write(0, 16, CONSOLE_COLORS);

    vga_set_fontopts(&CONSOLE_FOPTS);
    vga_upload_font(0, FONT_CHEESOS);
    vga_enable_cursor(false);
    vga_set_cursor(0, 0);

    console_set_attr(VGA_ATTR_WHITE, VGA_ATTR_BLACK);
    console_clear();
}

void console_clear(void) {
    vga_clear_text(' ', CONSOLE_STATE.attr);
    CONSOLE_STATE.row = 0;
    CONSOLE_STATE.col = 0;
}

void console_set_attr(uint8_t fg, uint8_t bg) {
    CONSOLE_STATE.attr = vga_make_attr(fg, bg);
}

void console_putchar(char c) {
    uint8_t w = vga_get_width_chars();
    uint8_t h = vga_get_height_chars();

    if (c == '\t') {
        uint8_t spaces = CONSOLE_STATE.col % TAB_SIZE;
        if (spaces == 0) {
            spaces = TAB_SIZE;
        }

        for (uint8_t i = 0; i < spaces; ++i) {
            console_putchar(' ');
        }
        return;
    } else if (c == '\n' || CONSOLE_STATE.col >= w) {
        CONSOLE_STATE.col = 0;
        if (CONSOLE_STATE.row >= h) {
            console_scroll(1);
        } else {
            ++CONSOLE_STATE.row;
        }
    }

    if (c != '\n') {
        vga_write_char(CONSOLE_STATE.col, CONSOLE_STATE.row, c, CONSOLE_STATE.attr);
        ++CONSOLE_STATE.col;
    }
}

void console_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        console_putchar(data[i]);
    }
}

void console_print(const char* str) {
    while (*str) {
        console_putchar(*str++);
    }
}

static int console_cprintf_cbk(void* ctx, const char* data, size_t size) {
    console_write(data, size);
    return 0;
}

void console_vprintf(const char* format, va_list args) {
    vcprintf(&console_cprintf_cbk, NULL, format, args);
}

void console_printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vcprintf(&console_cprintf_cbk, NULL, format, args);
    va_end(args);
}

void console_scroll(uint8_t rows) {
    vga_scroll_text(' ', CONSOLE_STATE.attr, rows);
}

void console_log_sink(void* context, enum log_level level, const char* file, unsigned line, const char* format, va_list args) {
    (void) context;

    uint8_t orig_attr = CONSOLE_STATE.attr;
    console_set_attr(LOG_LEVEL_COLORS[level], VGA_ATTR_BLACK);
    console_print(LOG_LEVEL_NAMES[level]);

    console_set_attr(VGA_ATTR_GRAY, VGA_ATTR_BLACK);
    console_printf(" %s:%u: ", file, line);

    console_set_attr(VGA_ATTR_WHITE, VGA_ATTR_BLACK);
    console_vprintf(format, args);
    console_putchar('\n');
    CONSOLE_STATE.attr = orig_attr;
}
