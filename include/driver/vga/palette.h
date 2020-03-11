#ifndef _CHEESOS2_DRIVER_VGA_PALETTE_H
#define _CHEESOS2_DRIVER_VGA_PALETTE_H

#include <stdint.h>
#include <stddef.h>

#define VGA_PALETTE_SIZE (16)

typedef struct {
    uint8_t overscan_index;
    uint8_t palette_indices[VGA_PALETTE_SIZE];
} vga_palette;

void vga_set_palette(const vga_palette* palette);

typedef struct {
    uint8_t red, green, blue;
} vga_dac_color;

#define VGA_DAC_NUM_ENTRIES (256)

void vga_dac_write_r3g3b2();
void vga_dac_write(uint8_t offset, size_t size, const vga_dac_color* colors);

#endif
