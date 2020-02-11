#include <stdint.h>
#include <stddef.h>

#include "vga/vga.h"
#include "memory/gdt.h"
#include "interrupt/idt.h"
#include "core/multiboot.h"
#include "pci/pci.h"
#include "libc/stdio.h"
#include "core/io.h"

void print_hex_char(uint8_t chr) {
    if (chr < 10) {
        vga_putchar(chr + '0');
    } else {
        vga_putchar(chr + 'A' - 10);
    }
}

void print_hex8(uint8_t value) {
    vga_print("0x");
    print_hex_char((value >> 4) & 0xF);
    print_hex_char(value & 0xF);
}

void print_hex16(uint16_t value) {
    vga_print("0x");
    print_hex_char((value >> 12) & 0xF);
    print_hex_char((value >> 8) & 0xF);
    print_hex_char((value >> 4) & 0xF);
    print_hex_char(value & 0xF);
}

void print_hex32(uint32_t value) {
    vga_print("0x");
    print_hex_char((value >> 28) & 0xF);
    print_hex_char((value >> 24) & 0xF);
    print_hex_char((value >> 20) & 0xF);
    print_hex_char((value >> 16) & 0xF);
    print_hex_char((value >> 12) & 0xF);
    print_hex_char((value >> 8) & 0xF);
    print_hex_char((value >> 4) & 0xF);
    print_hex_char(value & 0xF);
}

void report_pci(struct pci_device device, uint16_t vendor_id) {
    const uint16_t device_id = pci_config_read16(device, PCI_OFFSET_DEVICE_ID);
    const uint8_t class = pci_config_read8(device, PCI_OFFSET_CLASS);
    const uint8_t subclass = pci_config_read8(device, PCI_OFFSET_SUBCLASS);
    const uint8_t prog_if = pci_config_read8(device, PCI_OFFSET_PROG_IF);

    vga_putchar(device.bus + '0');
    vga_putchar(':');
    vga_putchar(device.slot + '0');
    vga_putchar('.');
    vga_putchar(device.function + '0');
    vga_putchar(' ');

    print_hex16(vendor_id);
    vga_putchar(' ');
    print_hex16(device_id);
    vga_print("  ");
    print_hex8(class);
    vga_print("     ");
    print_hex8(subclass);
    vga_print("    ");
    print_hex8(prog_if);
    vga_putchar('\n');
}

void kernel_main(multiboot_info* multiboot) {
    vga_init();
    gdt_init();
    idt_init();

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
    vga_print("pci devices:\n");
    vga_print("b:s.f vendor device class subclass prog-if\n");
    pci_scan(&report_pci);

    printf("%i\n", printf("test %u auwies, %z%% autistisch, de %s stiene dr weer schuune bie of zo, het antwoord is %c, ik ben %x, ook wel %X, vanbinnen. Ik heb %i kilo\'s appels.\n", 20, 100, "bieten", 'B', 57005, 57005, -285));
    
    printf("Shit werkte t/m hier");
}
