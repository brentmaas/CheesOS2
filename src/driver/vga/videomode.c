#include "driver/vga/videomode.h"
#include "driver/vga/ports.h"

const vga_videomode VGA_VIDEOMODE_640x480x16 = {
    .horizontal_timings = {
        .active_area = 640,
        .overscan_front = 8,
        .blanking_front = 16,
        .retrace = 96,
        .blanking_back = 32,
        .overscan_back = 8
    },
    .vertical_timings = {
        .active_area = 480,
        .overscan_front = 8,
        .blanking_front = 24,
        .retrace = 2,
        .blanking_back = 2,
        .overscan_back = 8
    }
};

void vga_set_videomode(const vga_videomode* mode) {
    // TODO: disable_display
    // TODO: unlock_crtc
    // TODO: load_registers
    // TODO: clear_screen
    // TODO: load_fonts
    // TODO: lock_crtc
    // TODO: enable_display
}
