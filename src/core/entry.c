#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#include "vga/vga.h"
#include "memory/gdt.h"
#include "interrupt/idt.h"
#include "core/multiboot.h"
#include "pci/pci.h"
#include "core/io.h"
#include "interrupt/pic.h"

#include "driver/vga/videomode.h"

void report_pci(struct pci_device device, uint16_t vendor_id) {
    const uint16_t device_id = pci_config_read16(device, PCI_OFFSET_DEVICE_ID);
    const uint8_t class = pci_config_read8(device, PCI_OFFSET_CLASS);
    const uint8_t subclass = pci_config_read8(device, PCI_OFFSET_SUBCLASS);
    const uint8_t prog_if = pci_config_read8(device, PCI_OFFSET_PROG_IF);

    printf(
        "%u:%u.%u 0x%04X 0x%04X  0x%02X     0x%02X    0x%02X\n",
        device.bus,
        device.slot,
        device.function,
        vendor_id,
        device_id,
        class,
        subclass,
        prog_if
    );
}

void initialize_vga() {
    vga_set_videomode(&VGA_VIDEOMODE_640x480x16);
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

    vga_print("Probing for PCI mechanism #1... ");
    bool has_mech1 = pci_probe_mech1();
    vga_print(has_mech1 ? "Yes\n" : "No\n");

    vga_print("Probing for PCI mechanism #2... ");
    bool has_mech2 = pci_probe_mech2();
    vga_print(has_mech2 ? "Yes\n" : "No\n");

    if (has_mech1) {
        vga_print("pci devices:\n");
        vga_print("b:s.f vendor device class subclass prog-if\n");
        pci_scan(&report_pci);
    }

    initialize_vga();
}
