#include <stdint.h>
#include <stddef.h>

#include "vga/vga.h"
#include "memory/gdt.h"
#include "core/multiboot.h"

void kernel_main(multiboot_info* multiboot) {
    vga_init();
    gdt_init();

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
}