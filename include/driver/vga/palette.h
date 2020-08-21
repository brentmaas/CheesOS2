#ifndef _CHEESOS2_DRIVER_VGA_PALETTE_H
#define _CHEESOS2_DRIVER_VGA_PALETTE_H

#include <stdint.h>
#include <stddef.h>

#define VGA_PALETTE_SIZE (16)

struct vga_palette {
    uint8_t overscan_index;
    uint8_t palette_indices[VGA_PALETTE_SIZE];
};

void vga_set_palette(const struct vga_palette* palette);

struct vga_dac_color {
    uint8_t red, green, blue;
};

#define VGA_DAC_NUM_ENTRIES (256)

void vga_dac_write_r3g3b2();
void vga_dac_write(uint8_t offset, size_t size, const struct vga_dac_color* colors);

#endif
