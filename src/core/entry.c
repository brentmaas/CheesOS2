#include <stdint.h>
#include <stddef.h>

#include "vga/vga.h"

void kernel_main() {
    vga_init();

    vga_print("OPPERPYTHON\n");
    vga_color(VGA_COLOR_GREEN, VGA_COLOR_RED);
    vga_print("IS\n");
    vga_color(VGA_COLOR_BLUE, VGA_COLOR_WHITE);
    vga_print("GOED\n");
}
