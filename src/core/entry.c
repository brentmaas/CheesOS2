#include <stdint.h>
#include <stddef.h>

#include "vga/vga.h"
#include "memory/gdt.h"
#include "interrupt/idt.h"
#include "core/multiboot.h"
#include "core/io.h"
#include "interrupt/pic.h"

#include "driver/vga/videomode.h"
#include "driver/vga/palette.h"
#include "driver/vga/text.h"
#include "res/fonts.h"

#include "driver/console/console.h"
#include "driver/serial/serial.h"

#include "ps2/controller.h"

#include "debug/log.h"
#include "debug/memdump.h"

static void initialize_vga_new() {
    vga_set_videomode(&VGA_VIDEOMODE_640x480, VGA_MODE_TEXT);
    const struct vga_dac_color dos_colors[] = {
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

    vga_dac_write(0, 16, dos_colors);

    const struct vga_font_options fontopts = {
        .text_height = 16,
        .cursor = {
            .start = 14,
            .end = 16
        },
        .enable_blink = false,
        .enable_line_graphics = false
    };

    vga_set_fontopts(&fontopts);
    vga_upload_font(0, FONT_CHEESOS);

    vga_clear_text(' ', 0x0F);
    vga_enable_cursor(false);
    vga_set_cursor(0, 0);
}

static int sink_serial_cprintf_cbk(void* context, const char* data, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        serial_busy_write(SERIAL_PORT_1, data[i]);
    }
    return 0;
}

static void sink_serial(void* context, enum log_level level, const char* file, unsigned line, const char* format, va_list args) {
    cprintf(sink_serial_cprintf_cbk, NULL, "%s %s:%u: ", LOG_LEVEL_NAMES[level], file, line);
    vcprintf(sink_serial_cprintf_cbk, NULL, format, args);
    serial_busy_write(SERIAL_PORT_1, '\n');
}

static void sink_serial_and_vga(void* context, enum log_level level, const char* file, unsigned line, const char* format, va_list args) {
    sink_serial(context, level, file, line, format, args);

    const enum vga_color_type log_level_colors[] = {
        [LOG_LEVEL_DEBUG] = VGA_COLOR_LIGHT_GREEN,
        [LOG_LEVEL_INFO] = VGA_COLOR_LIGHT_BLUE,
        [LOG_LEVEL_WARN] = VGA_COLOR_LIGHT_BROWN,
        [LOG_LEVEL_ERROR] = VGA_COLOR_LIGHT_RED,
    };

    vga_color(log_level_colors[level], VGA_COLOR_BLACK);
    vga_print(LOG_LEVEL_NAMES[level]);
    vga_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);

    vga_printf(" %s:%u: ", file, line);
    vga_vprintf(format, args);
    vga_putchar('\n');
}

void kernel_main(const struct multiboot_info* multiboot) {
    serial_init(SERIAL_PORT_1, ((struct serial_init_info) {
        .data_size = SERIAL_DATA_SIZE_8_BITS,
        .stop_bits = SERIAL_STOP_BITS_ONE,
        .parity = SERIAL_PARITY_NONE,
        .enable_break = false,
        .baudrate_divisor = SERIAL_BAUDRATE_DIVISOR(9600)
    }));

    log_set_sink(sink_serial, NULL);
    log_info("Initializing VGA");
    initialize_vga_new();
    vga_init();
    log_set_sink(sink_serial_and_vga, NULL);

    log_info("Initializing GDT");
    gdt_init();

    log_info("Initializing IDT");
    idt_init();
    pic_remap(0x20, 0x28);
    pic_set_mask(0xFFFF); //Disable PIC
    idt_enable();

    if (multiboot->flags & MULTIBOOT_FLAG_BOOT_LOADER_NAME) {
        log_info("Booted from %s", multiboot->boot_loader_name);
    }
    if (multiboot->flags & MULTIBOOT_FLAG_CMDLINE) {
        log_info("Booted with command line \"%s\"", multiboot->cmdline);
    }

    if (ps2_controller_init()) {
        log_error("PS2 initialization failed");
        return;
    }

    vga_print("OPPERPYTHON\n");
    vga_color(VGA_COLOR_GREEN, VGA_COLOR_RED);
    vga_print("IS\n");
    vga_color(VGA_COLOR_BLUE, VGA_COLOR_WHITE);
    vga_print("GOED\n");
    vga_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);

    console();
}
