#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

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

void initialize_vga_new() {
    vga_set_videomode(&VGA_VIDEOMODE_640x480, VGA_MODE_TEXT);
    const vga_dac_color dos_colors[] = {
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

    const vga_font_options fontopts = {
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
}

void kernel_main(multiboot_info* multiboot) {
    vga_init();
    gdt_init();
    idt_init();
    pic_remap(0x20, 0x28);
    pic_set_mask(0xFFFF); //Disable PIC
    idt_enable();

    if(multiboot->flags & MULTIBOOT_FLAG_BOOT_LOADER_NAME) {
        vga_print("Booted from ");
        vga_print(multiboot->boot_loader_name);
        vga_print("\n");
    }
    if(multiboot->flags & MULTIBOOT_FLAG_CMDLINE) {
        vga_print("Booted with command line ");
        vga_print(multiboot->cmdline);
        vga_print("\n");
    }

    vga_print("OPPERPYTHON\n");
    vga_color(VGA_COLOR_GREEN, VGA_COLOR_RED);
    vga_print("IS\n");
    vga_color(VGA_COLOR_BLUE, VGA_COLOR_WHITE);
    vga_print("GOED\n");

    vga_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);

    initialize_vga_new();

    vga_clear_text(' ', 0x0F);

    vga_enable_cursor(true);
    vga_set_cursor(79, 29);

    vga_write_str(0, 0, "OPPERPYTHON", vga_make_attr(VGA_ATTR_WHITE, VGA_ATTR_BLUE));
    vga_write_str(0, 1, "IS", vga_make_attr(VGA_ATTR_WHITE, VGA_ATTR_GREEN));
    vga_write_str(0, 2, "GOED", vga_make_attr(VGA_ATTR_WHITE, VGA_ATTR_RED));
    vga_write_str(0, 3, "\x90\x91\x91\x91\x91\x91\x91\x91\x91\x92", vga_make_attr(VGA_ATTR_GREEN, VGA_ATTR_BLACK));

    vga_write_str(0, 29, "oef", vga_make_attr(VGA_ATTR_WHITE, VGA_ATTR_BLACK));
    
    console_init();
    console_loop();
}
