#include "driver/vga/palette.h"
#include "driver/vga/io.h"
#include "driver/vga/util.h"
#include "core/io.h"
#include "interrupt/idt.h"

void vga_set_palette(const vga_palette* palette) {
    for (uint8_t i = 0; i < VGA_PALETTE_SIZE; ++i) {
        vga_prepare_atc(i);
        VGA_WRITE(VGA_PORT_ATC, palette->palette_indices[i]);
    }

    vga_prepare_atc(VGA_IND_ATC_OVERSCAN_COLOR);
    VGA_WRITE(VGA_PORT_ATC, palette->overscan_index);

    vga_sync_atc();
    VGA_WRITE(VGA_PORT_ATC, ((vga_atc_address) {
        .attribute_address = 0,
        .lock_palette = true
    }));
}

void vga_dac_write_r3g3b2() {
    idt_disable();
    io_out8(VGA_PORT_DAC_ADDR_WRITE, 0);

    const uint8_t blue_bits = 2;
    const uint8_t green_bits = 3;
    const uint8_t red_bits = 3;
    const uint8_t dac_max = (1 << 6) - 1; // The DAC only has 6-bit color values

    for (uint8_t red = 0; red < 8; ++red) {
        uint8_t dac_red = dac_max * red / ((1 << red_bits) - 1);

        for (uint8_t green = 0; green < (1 << green_bits); ++green) {
            uint8_t dac_green = dac_max * green / ((1 << green_bits) - 1);

            for (uint8_t blue = 0; blue < (1 << blue_bits); ++blue) {
                uint8_t dac_blue = dac_max * blue / ((1 << blue_bits) - 1);

                io_out8(VGA_PORT_DAC_DATA, dac_red);
                io_out8(VGA_PORT_DAC_DATA, dac_green);
                io_out8(VGA_PORT_DAC_DATA, dac_blue);
            }
        }
    }

    idt_enable();
}

void vga_dac_write(uint8_t offset, size_t size, const vga_dac_color* colors) {
    idt_disable();
    io_out8(VGA_PORT_DAC_ADDR_WRITE, offset);

    for (size_t i = 0; i < size && offset + i < VGA_DAC_NUM_ENTRIES; ++i) {
        io_out8(VGA_PORT_DAC_DATA, colors[i].red);
        io_out8(VGA_PORT_DAC_DATA, colors[i].green);
        io_out8(VGA_PORT_DAC_DATA, colors[i].blue);
    }

    idt_enable();
}
