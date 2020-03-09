#include "driver/vga/io.h"
#include "driver/vga/registers.h"
#include <stdbool.h>

void vga_sync_atc() {
    (void) io_in8(VGA_PORT_INPUT_STATUS_1_COLOR);
}

void vga_prepare_atc(uint8_t index) {
    vga_sync_atc();

    VGA_WRITE(VGA_PORT_ATC, ((vga_atc_address) {
        .attribute_address = index,
        .lock_palette = false
    }));
}
