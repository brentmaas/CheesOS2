#include "driver/vga/io.h"
#include "driver/vga/registers.h"
#include <stdbool.h>

void vga_sync_atc() {
    (void) io_in8(VGA_PORT_INPUT_STATUS_1_COLOR);
}

void vga_prepare_atc(uint8_t index, bool lock_palette) {
    vga_sync_atc();

    VGA_WRITE(VGA_PORT_ATC, ((vga_atc_address) {
        .attribute_address = index,
        .lock_palette = lock_palette
    }));
}

void vga_read_io_state(vga_io_state* state) {
    state->grc = io_in8(VGA_PORT_GRC_ADDR);
    state->seq = io_in8(VGA_PORT_SEQ_ADDR);
    state->crtc = io_in8(VGA_PORT_CRTC_COLOR_ADDR);
    state->atc = io_in8(VGA_PORT_ATC);
}

void vga_write_io_state(const vga_io_state* state) {
    io_out8(VGA_PORT_GRC_ADDR, state->grc);
    io_out8(VGA_PORT_SEQ_ADDR, state->seq);
    io_out8(VGA_PORT_CRTC_COLOR_ADDR, state->crtc);

    vga_sync_atc();
    io_out8(VGA_PORT_ATC, state->atc);
}
