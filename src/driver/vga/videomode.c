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
    .enable_graphics = true,
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

static void dump_registers() {
    printf("misc:\n    %02X\nsequencer:\n    ", io_in8(VGA_PORT_MISC_READ));
    for (size_t i = 0; i < VGA_NUM_SEQ_INDICES; ++i) {
        io_out8(VGA_PORT_SEQ_ADDR, i);
        printf("%02X ", io_in8(VGA_PORT_SEQ_DATA));
    }

    printf("\ncrtc:");
    for (size_t i = 0; i < VGA_NUM_CRTC_INDICES; ++i) {
        if (i % 8 == 0) printf("\n    ");
        io_out8(VGA_PORT_CRTC_COLOR_ADDR, i);
        printf("%02X ", io_in8(VGA_PORT_CRTC_COLOR_DATA));
    }

    printf("\ngraphics:");
    for (size_t i = 0; i < VGA_NUM_GRC_INDICES; ++i) {
        if (i % 8 == 0) printf("\n    ");
        io_out8(VGA_PORT_GRC_ADDR, i);
        printf("%02X ", io_in8(VGA_PORT_GRC_DATA));
    }

    printf("\natc:");
    for (size_t i = 0; i < VGA_NUM_ATC_INDICES; ++i) {
        if (i % 8 == 0) printf("\n    ");
        sync_atc();
        VGA_WRITE(VGA_PORT_ATC, ((vga_atc_address) {
            .attribute_address = i,
            .lock_palette = true
        }));
        printf("%02X ", io_in8(VGA_PORT_ATC_READ));
    }
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

static uint8_t color_depth_pixels_per_address(vga_color_depth color_depth) {
    uint8_t bits_per_plane_address = 8;
    uint8_t bits_per_pixel = VGA_COLOR_DEPTH_REQUIRED_BITS(color_depth);
    uint8_t depth_planes[] = {
        [VGA_COLOR_DEPTH_2_COLOR] = 1,
        [VGA_COLOR_DEPTH_4_COLOR] = 2,
        [VGA_COLOR_DEPTH_16_COLOR] = 4,
        [VGA_COLOR_DEPTH_256_COLOR] = 4
    };

    return bits_per_plane_address * depth_planes[color_depth] / bits_per_pixel;
}

// Assumes CRT registers are unlocked (and leaves them unlocked).
static void set_crtc_registers(const vga_videomode* mode) {
    // Set horizontal values
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
            .horiz_blanking_end_high = (h_blanking_end >> 5) & 0x1
        }));
    }

    // Set vertical values
    {
        // Considerations for these values, for which different documentation
        // and examples don't agree:
        // * v_display_end is the total number of scan lines in the active display
        // minus 1.
        // * v_blanking_start is 'the horizontal scan line count at which the "vertical
        // blanking" signal becomes active minus 1.
        // * There is no explicit mention 1 should also be subtracted from v_retrace_start, however,
        // when this signal becomes active is otherwise similar described to the vertical blanking start
        // value in both FreeVGA and the IBM VGA manual, so 1 is subtracted.
        // * According to the IBM VGA manual, the end vertical blanking value is the value in the
        // start vertical blanking register plus signal width minus 1. The start vertical blanking
        // register was already subtracted one from, so this yields v_blanking_end - 2.
        // * Differently from the vertical blanking end value, the IBM VGA manual does not mention
        // subtracting one from the value in the vertical retrace start register (instead just adding the
        // pulse width to the value in the value in the vertical retrace start register).
        // * According to the IBM VGA manual, the vertical total register contains
        // "the number of horizontal rasters scans (including vertical retrace) minus 2"

        uint16_t v_display_end = mode->vertical_timings.active_area;
        uint16_t v_blanking_start = v_display_end + mode->vertical_timings.overscan_back;
        uint16_t v_retrace_start = v_blanking_start + mode->vertical_timings.blanking_back;
        uint16_t v_retrace_end = v_retrace_start + mode->vertical_timings.retrace;
        uint16_t v_blanking_end = v_retrace_end + mode->vertical_timings.blanking_front;
        uint16_t v_total = v_blanking_end + mode->vertical_timings.overscan_front;
        uint16_t line_compare = 0x3FF;

        --v_display_end;
        --v_blanking_start;
        --v_retrace_start;
        v_blanking_end -= 2;
        --v_retrace_end;
        v_total -= 2;

        io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_VERT_TOTAL);
        VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, (uint8_t) v_total);

        io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_OVERFLOW);
        VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, ((vga_crtc_overflow) {
            .vert_total_8 = (v_total >> 8) & 0x1,
            .vert_display_end_8 = (v_display_end >> 8) & 0x1,
            .vert_retrace_start_8 = (v_retrace_start >> 8) & 0x1,
            .vert_blanking_start_8 = (v_blanking_start >> 8) & 0x1,
            .line_compare_8 = (line_compare >> 8) & 0x1,
            .vert_total_9 = (v_total >> 9) & 0x1,
            .vert_display_end_9 = (v_display_end >> 9) & 0x1,
            .vert_retrace_start_9 = (v_retrace_start >> 9) & 0x1
        }));

        io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_MAXIMUM_SCAN_LINE);
        VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, ((vga_crtc_maximum_scan_line) {
            .maximum_scan_line = 0x0F, // TODO: Allow for finer control
            .vert_blanking_start_9 = (v_blanking_start >> 9) & 0x1,
            .line_compare_9 = (line_compare >> 9) & 0x1,
            .enable_double_scan = 0
        }));

        io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_VERT_RETRACE_START);
        VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, (uint8_t) v_retrace_start);

        io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_VERT_RETRACE_END);
        VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, ((vga_crtc_vert_retrace_end) {
            .vert_retrace_end = v_retrace_end & 0xF,
            .bandwidth = 0,
            .protect = false
        }));

        io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_VERT_DISPLAY_END);
        VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, (uint8_t) v_display_end);

        io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_VERT_BLANKING_START);
        VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, (uint8_t) v_blanking_start);

        io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_VERT_BLANKING_END);
        VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, (uint8_t) v_blanking_end);

        io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_LINE_COMPARE);
        VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, (uint8_t) line_compare);
    }

    vga_address_mode address_mode = VGA_ADDRESS_MODE_BYTES;

    // Set the other display-generation related values
    io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_MODE);
    VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, ((vga_crtc_mode) {
        .map13 = true,
        .map14 = true,
        .enable_half_scanline_clock = false,
        .enable_half_memory_clock = false,
        .address_wrap_select = true,
        .disable_word_addressing = address_mode != VGA_ADDRESS_MODE_WORDS,
        .enable_sync = true
    }));

    uint8_t memory_address_size = VGA_ADDRESS_MODE_SIZE(address_mode);
    uint8_t pixels_per_address = mode->enable_graphics ? color_depth_pixels_per_address(mode->color_depth) : 1;
    uint8_t offset = mode->horizontal_timings.active_area / (pixels_per_address * memory_address_size * 2);

    io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_OFFSET);
    VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, offset);

    io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_UNDERLINE_LOCATION);
    VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, ((vga_crtc_underline_location) {
        .underline_location = 0,
        .enable_quarter_memory_clock = false,
        .enable_dword_addressing = address_mode == VGA_ADDRESS_MODE_DWORDS,
    }));

    // Finally set the other CRTC registers
    io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_PRESET_ROW_SCAN);
    VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, ((vga_crtc_preset_row_scan) {
        .preset_row_scan = 0,
        .byte_panning = 0
    }));

    // TODO: Move cursor code somewhere else?
    io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_CURSOR_START);
    VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, ((vga_crtc_cursor_start) {
        .cursor_scan_line_start = 0,
        .disable_cursor = true
    }));

    io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_CURSOR_END);
    VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, ((vga_crtc_cursor_end) {
        .cursor_scan_line_end = 0xF, // TODO: Set to value in maximum scan line
        .cursor_skew = 0
    }));

    io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_START_ADDRESS_HIGH);
    VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, (uint8_t) 0);
    io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_START_ADDRESS_LOW);
    VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, (uint8_t) 0);

    // TODO: Also cursor code
    io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_CURSOR_LOCATION_HIGH);
    VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, (uint8_t) 0);
    io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_CURSOR_LOCATION_LOW);
    VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, (uint8_t) 0);
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
    set_crtc_registers(mode);
    unblank_and_lock();

    dump_registers();
}
