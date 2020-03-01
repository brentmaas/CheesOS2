#include "driver/vga/videomode.h"
#include "driver/vga/ports.h"
#include "core/io.h"
#include <stdio.h>

const vga_videomode VGA_VIDEOMODE_640x480x16 = {
    .horizontal_timings = {
        .active_area = 640,
        .overscan_back = 8,
        .blanking_back = 32,
        .retrace = 96,
        .blanking_front = 16,
        .overscan_front = 8
    },
    .vertical_timings = {
        .active_area = 480,
        .overscan_back = 8,
        .blanking_back = 2,
        .retrace = 2,
        .blanking_front = 24,
        .overscan_front = 8
    },
    .dot_mode = VGA_DOT_MODE_8_DPC,
    .clock_speed = VGA_CLOCK_SPEED_25MHZ,
    .color_depth = VGA_COLOR_DEPTH_16_COLOR
};

#define VGA_READ(port, reg_ptr)                                 \
    do {                                                        \
        _Static_assert(sizeof(uint8_t) == sizeof(*reg_ptr));    \
        *(reg_ptr) = (union{                                    \
            uint8_t a;                                          \
            typeof(*(reg_ptr)) b;                               \
        }){.a = io_in8(port)}.b;                                \
    } while(0)

#define VGA_WRITE(port, reg)                                    \
    do {                                                        \
        _Static_assert(sizeof(uint8_t) == sizeof(reg));         \
        io_out8(port, (union{                                   \
            uint8_t a;                                          \
            typeof(reg) b;                                      \
        }){.b = (reg)}.a);                                      \
    } while(0)

static void sync_atc() {
    (void) io_in8(VGA_PORT_INPUT_STATUS_1_COLOR);
}

static void blank_and_unlock() {
    vga_crtc_horiz_blanking_end h_blanking_end;
    io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_HORIZ_BLANKING_END);
    VGA_READ(VGA_PORT_CRTC_COLOR_DATA, &h_blanking_end);
    h_blanking_end.enable_vert_retrace_access = true;
    VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, h_blanking_end);

    vga_crtc_vert_retrace_end v_retrace_end;
    io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_VERT_RETRACE_END);
    VGA_READ(VGA_PORT_CRTC_COLOR_DATA, &v_retrace_end);
    v_retrace_end.protect = false;
    VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, v_retrace_end);
}

static void unblank_and_lock() {
    vga_crtc_vert_retrace_end v_retrace_end;
    io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_VERT_RETRACE_END);
    VGA_READ(VGA_PORT_CRTC_COLOR_DATA, &v_retrace_end);
    v_retrace_end.protect = true;
    VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, v_retrace_end);

    vga_atc_address lock_palette = {.lock_palette = true};
    sync_atc();
    VGA_WRITE(VGA_PORT_ATC, lock_palette);
}

// Set the CRT registers. Assumes CRT registers are unlocked.
static void set_display_generation(const vga_videomode* mode) {
    {
        uint8_t h_div = mode->dot_mode == VGA_DOT_MODE_9_DPC ? 9 : 8;

        uint8_t h_display_end = mode->horizontal_timings.active_area / h_div - 1;
        uint8_t h_blanking_start = h_display_end + mode->horizontal_timings.overscan_back / h_div;
        uint8_t h_retrace_start = h_blanking_start + mode->horizontal_timings.blanking_back / h_div;
        uint8_t h_retrace_end = h_retrace_start + mode->horizontal_timings.retrace / h_div;
        uint8_t h_blanking_end = h_retrace_end + mode->horizontal_timings.blanking_front / h_div;
        uint8_t h_total = h_blanking_end + mode->horizontal_timings.overscan_front / h_div + 1 - 5;

        io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_HORIZ_TOTAL);
        VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, h_total);

        io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_HORIZ_DISPLAY_END);
        VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, h_display_end);

        io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_HORIZ_BLANKING_START);
        VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, h_blanking_start);

        io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_HORIZ_BLANKING_END);
        VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, ((vga_crtc_horiz_blanking_end) {
             // lower 5 bits stored here, 6th bit stored in the retrace end register
            .horiz_blanking_end_low = h_blanking_end & 0x1F,
            .enable_display_skew = false,
            .enable_vert_retrace_access = true
        }));

        io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_HORIZ_RETRACE_START);
        VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, h_retrace_start);

        io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_HORIZ_RETRACE_END);
        VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, ((vga_crtc_horiz_retrace_end) {
            .horiz_retrace_end = h_retrace_end & 0x1F, // lower 5 bits
            .horiz_retrace_skew = 0,
            .horiz_blanking_end_high = h_blanking_end >> 5
        }));
    }
}

void vga_set_videomode(const vga_videomode* mode) {
    // Make sure the right address space is selected
    // TODO: write the rest of misc
    VGA_WRITE(VGA_PORT_MISC_WRITE, ((vga_misc) {
        .io_emulation_mode = VGA_EMULATION_MODE_COLOR,
        .enable_vram_access = 1,
        .clock_select = mode->clock_speed,
        .odd_even_select_high_page = false,
        .hsync_polarity = 1, // negative polarity
        .vsync_polarity = 1,
    }));

    blank_and_unlock();
    // TODO: Display memory access
    // TODO: Display sequencing
    // TODO: Cursor
    // TODO: Attributes
    // TODO: DAC
    set_display_generation(mode);
    unblank_and_lock();
}
