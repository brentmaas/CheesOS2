#include "driver/vga/videomode.h"
#include "driver/vga/io.h"
#include "driver/vga/util.h"
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

static struct {
    uint16_t width_pixels;
    uint16_t height_pixels;
    uint8_t width_chars;
} VGA_SETTINGS = {};

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
        vga_sync_atc();
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
    vga_sync_atc();
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
    uint8_t h_div = mode->dot_mode == VGA_DOT_MODE_9_DPC ? 9 : 8;

    // Set horizontal values
    {

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
            .maximum_scan_line = mode->enable_graphics ? 0 : VGA_TEXTMODE_CHARACTER_HEIGHT,
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

    vga_address_mode address_mode = mode->enable_graphics ?
        VGA_ADDRESS_MODE_BYTES : VGA_ADDRESS_MODE_WORDS;

    // Set the other display-generation related values
    io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_MODE);
    VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, ((vga_crtc_mode) {
        .map13 = true,
        .map14 = true,
        .enable_half_scanline_clock = false,
        .enable_half_memory_clock = false,
        .address_wrap_select = 1,
        .disable_word_addressing = address_mode != VGA_ADDRESS_MODE_WORDS,
        .enable_sync = true
    }));

    uint8_t memory_address_size = VGA_ADDRESS_MODE_SIZE(address_mode);
    uint8_t offset;

    if (mode->enable_graphics) {
        // Equation from FreeVGA
        uint8_t pixels_per_address = color_depth_pixels_per_address(mode->color_depth);
        offset = mode->horizontal_timings.active_area / (pixels_per_address * memory_address_size * 2);
    } else {
        // The value of the offset field is multiplied by `2 * memory_address_size`
        // The width in bytes of a column is `columns * memory_address_size`
        // this yields `columns * memory_address_size / (2 * memory_address_size) =
        // columns / 2`.
        uint8_t columns = mode->horizontal_timings.active_area / h_div;
        offset = columns / 2;
    }

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

    io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_START_ADDRESS_HIGH);
    VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, (uint8_t) 0);
    io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_START_ADDRESS_LOW);
    VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, (uint8_t) 0);
}

static void reset_cursor() {
    io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_CURSOR_START);
    VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, ((vga_crtc_cursor_start) {
        .cursor_scan_line_start = 0,
        .disable_cursor = true
    }));

    io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_CURSOR_END);
    VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, ((vga_crtc_cursor_end) {
        .cursor_scan_line_end = VGA_TEXTMODE_CHARACTER_HEIGHT - 1,
        .cursor_skew = 0
    }));

    io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_CURSOR_LOCATION_HIGH);
    VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, (uint8_t) 0);
    io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_CURSOR_LOCATION_LOW);
    VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, (uint8_t) 0);
}

static void set_grc_registers(const vga_videomode* mode) {
    io_out8(VGA_PORT_GRC_ADDR, VGA_IND_GRC_SET_RESET);
    VGA_WRITE(VGA_PORT_GRC_DATA, ((vga_grc_set_reset) {
        .planes = 0
    }));

    io_out8(VGA_PORT_GRC_ADDR, VGA_IND_GRC_ENABLE_SET_RESET);
    VGA_WRITE(VGA_PORT_GRC_DATA, ((vga_grc_enable_set_reset) {
        .planes = 0
    }));

    io_out8(VGA_PORT_GRC_ADDR, VGA_IND_GRC_COLOR_COMPARE);
    VGA_WRITE(VGA_PORT_GRC_DATA, ((vga_grc_color_compare) {
        .planes = 0
    }));

    io_out8(VGA_PORT_GRC_ADDR, VGA_IND_GRC_DATA_ROTATE);
    VGA_WRITE(VGA_PORT_GRC_DATA, ((vga_grc_data_rotate) {
        .rotate_count = 0,
        .operation = VGA_OP_NOP
    }));

    io_out8(VGA_PORT_GRC_ADDR, VGA_IND_GRC_READ_MAP);
    VGA_WRITE(VGA_PORT_GRC_DATA, ((vga_grc_read_map) {
        .plane = 0
    }));

    io_out8(VGA_PORT_GRC_ADDR, VGA_IND_GRC_MODE);
    VGA_WRITE(VGA_PORT_GRC_DATA, ((vga_grc_mode) {
        .write_mode = 0,
        .read_mode = 0,
        .enable_host_odd_even = !mode->enable_graphics, // Odd-even mode for reading
        .enable_shift_interleave = false,
        .enable_shift256 = false
    }));

    io_out8(VGA_PORT_GRC_ADDR, VGA_IND_GRC_MISC);
    VGA_WRITE(VGA_PORT_GRC_DATA, ((vga_grc_misc) {
        .enable_graphics_mode = mode->enable_graphics,
        .enable_chain_odd_even = false,
        .memory_map = mode->enable_graphics ?
            VGA_MEMORY_MAP_A0000_64K : VGA_MEMORY_MAP_B8000_32K
    }));

    io_out8(VGA_PORT_GRC_ADDR, VGA_IND_GRC_COLOR_DONT_CARE);
    VGA_WRITE(VGA_PORT_GRC_DATA, ((vga_grc_color_compare) {
        .planes = VGA_PLANE_ALL
    }));

    io_out8(VGA_PORT_GRC_ADDR, VGA_IND_GRC_BIT_MASK);
    VGA_WRITE(VGA_PORT_GRC_DATA, (uint8_t) 0xFF);
}

static void set_seq_registers(const vga_videomode* mode) {
    io_out8(VGA_PORT_SEQ_ADDR, VGA_IND_SEQ_RESET);
    VGA_WRITE(VGA_PORT_SEQ_DATA, ((vga_seq_reset) {
        .synchronous_reset = true,
        .asynchronous_reset = true
    }));

    io_out8(VGA_PORT_SEQ_ADDR, VGA_IND_SEQ_CLOCKING_MODE);
    VGA_WRITE(VGA_PORT_SEQ_DATA, ((vga_seq_clocking_mode) {
        .dot_mode = mode->dot_mode,
        .shift_load_rate = 0,
        .enable_half_dot_clock = false, // TODO: set for 320 and 360 resolutions
        .enable_shift_4 = false,
        .disable_display = false
    }));

    io_out8(VGA_PORT_SEQ_ADDR, VGA_IND_SEQ_MAP_MASK);
    VGA_WRITE(VGA_PORT_SEQ_DATA, ((vga_seq_map_mask) {
        // In text modes, planes 0 and 1 are used in odd/even mode
        // Graphics modes use planar mode
        .planes = VGA_PLANE_0_BIT | VGA_PLANE_1_BIT |
            (mode->enable_graphics ? 0 : (VGA_PLANE_2_BIT | VGA_PLANE_3_BIT))
    }));

    io_out8(VGA_PORT_SEQ_ADDR, VGA_IND_SEQ_CHARACTER_MAP_SELECT);
    VGA_WRITE(VGA_IND_SEQ_CHARACTER_MAP_SELECT, (uint8_t) 0);

    io_out8(VGA_PORT_SEQ_ADDR, VGA_IND_SEQ_MEMORY_MODE);
    VGA_WRITE(VGA_PORT_SEQ_DATA, ((vga_seq_memory_mode) {
        .extended_memory = true,
        .disable_odd_even = mode->enable_graphics,
        .enable_chain_4 = false
    }));
}

// Assumes display is blanked and unlocked.
static void set_atc_registers(const vga_videomode* mode) {
    // TODO: Allow more control
    for (uint8_t i = 0; i < 16; ++i) {
        vga_prepare_atc(i);
        VGA_WRITE(VGA_PORT_ATC, i);
    }

    vga_prepare_atc(VGA_IND_ATC_MODE_CONTROL);
    VGA_WRITE(VGA_PORT_ATC, ((vga_atc_mode_control) {
        .enable_graphics = mode->enable_graphics,
        .enable_monochrome_emulation = false,
        .enable_line_graphics = true,
        .enable_blink = false, // TODO: Allow more control
        .enable_bottom_pixel_pan = false,
        .enable_256_color = mode->color_depth == VGA_COLOR_DEPTH_256_COLOR,
        .enable_color_select = false
    }));

    vga_sync_atc();
    VGA_WRITE(VGA_PORT_ATC, ((vga_atc_address) {
        .attribute_address = VGA_IND_ATC_OVERSCAN_COLOR,
        .lock_palette = true
    }));

    vga_prepare_atc(VGA_IND_ATC_OVERSCAN_COLOR);
    VGA_WRITE(VGA_PORT_ATC, (uint8_t) 0); // TODO: More control

    vga_prepare_atc(VGA_IND_ATC_COLOR_PLANE_ENABLE);
    VGA_WRITE(VGA_PORT_ATC, ((vga_atc_color_plane_enable) {
        .planes = VGA_PLANE_ALL
    }));

    vga_prepare_atc(VGA_IND_ATC_HORIZ_PIXEL_PANNING);
    VGA_WRITE(VGA_PORT_ATC, ((vga_atc_horiz_pixel_panning) {
        .pixel_shift = 0
    }));

    vga_prepare_atc(VGA_IND_ATC_COLOR_SELECT);
    VGA_WRITE(VGA_PORT_ATC, ((vga_atc_color_select) {
        .color_select_5_4 = 0,
        .color_select_7_6 = 0
    }));
}

static void clear_vram(uint8_t planes) {
    const vga_memory_map map = VGA_MEMORY_MAP_A0000_64K;
    vga_plane_bits orig_mask = vga_mask_planes(planes);
    vga_memory_map orig_map = vga_map_memory(map);

    volatile uint8_t* vram = vga_memory_map_ptr(map);
    for (size_t i = 0; i < (1 << 16); ++i) {
        vram[i] = 0;
    }

    vga_map_memory(orig_map);
    vga_mask_planes(orig_mask);
}

// Debug graphics
static void set_pixel_16(uint16_t x, uint16_t y) {
    volatile uint8_t* vram = (volatile uint8_t*) 0xA0000;
    size_t byte = y * 80 + x / 8;
    size_t shift = 7 - x % 8;

    io_out8(VGA_PORT_GRC_ADDR, VGA_IND_GRC_BIT_MASK);
    io_out8(VGA_PORT_GRC_DATA, 1 << shift);
    (void) vram[byte]; // load latch
    vram[byte] = 0xFF;
    // TODO: What to do with shift?
}

static int abs(int x) {
    return x < 0 ? -x : x;
}

static void line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;

    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;

    int err = dx + dy;

    while (true) {
        set_pixel_16(x0, y0);
        if (x0 == x1 && y0 == y1) {
            break;
        }

        int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }

        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

static void draw_python() {
    const uint16_t python[] = {
        12, 64, 12, 65, 46, 23, 60, 17, 52, 37, 66, 31, 23, 51, 10, 61, 105, 53,
        92, 59, 92, 59, 81, 62, 81, 62, 70, 64, 70, 64, 61, 65, 61, 65, 52, 64,
        52, 64, 44, 63, 44, 63, 36, 60, 36, 60, 30, 57, 30, 57, 26, 54, 26, 54,
        22, 50, 22, 50, 19, 47, 19, 47, 17, 43, 17, 43, 17, 39, 17, 39, 17, 35,
        17, 35, 18, 32, 18, 32, 19, 28, 19, 28, 22, 24, 22, 24, 24, 21, 24, 21,
        27, 18, 27, 18, 30, 15, 30, 15, 34, 12, 34, 12, 37, 9, 37, 9, 41, 7, 41,
        7, 45, 5, 45, 5, 49, 3, 49, 3, 53, 2, 53, 2, 57, 1, 57, 1, 61, 0, 61, 0,
        65, 0, 65, 0, 69, 0, 69, 0, 73, 1, 73, 1, 77, 2, 77, 2, 81, 3, 81, 3,
        86, 5, 86, 5, 90, 7, 90, 7, 94, 9, 94, 9, 98, 12, 98, 12, 102, 15, 102,
        15, 106, 18, 106, 18, 111, 21, 111, 21, 115, 25, 115, 25, 119, 28, 119,
        28, 123, 32, 123, 32, 127, 36, 127, 36, 131, 40, 131, 40, 135, 44, 135,
        44, 140, 48, 140, 48, 144, 52, 144, 52, 148, 57, 148, 57, 152, 61, 152,
        61, 156, 65, 156, 65, 161, 69, 161, 69, 165, 72, 165, 72, 169, 76, 169,
        76, 174, 79, 174, 79, 179, 82, 179, 82, 184, 85, 184, 85, 189, 87, 189,
        87, 194, 89, 194, 89, 199, 90, 199, 90, 205, 91, 205, 91, 211, 92, 211,
        92, 216, 92, 216, 92, 222, 92, 222, 92, 228, 92, 228, 92, 233, 91, 233,
        91, 239, 91, 239, 91, 244, 89, 244, 89, 249, 88, 249, 88, 254, 87, 254,
        87, 258, 85, 258, 85, 263, 84, 263, 84, 267, 82, 267, 82, 271, 80, 271,
        80, 276, 78, 276, 78, 280, 76, 280, 76, 284, 74, 284, 74, 288, 72, 288,
        72, 292, 70, 292, 70, 297, 68, 297, 68, 301, 66, 301, 66, 305, 64, 305,
        64, 310, 62, 310, 62, 314, 60, 314, 60, 319, 59, 319, 59, 324, 58, 324,
        58, 329, 56, 329, 56, 334, 56, 334, 56, 340, 55, 340, 55, 345, 54, 345,
        54, 351, 54, 351, 54, 357, 54, 357, 54, 363, 54, 363, 54, 368, 55, 368,
        55, 374, 55, 374, 55, 379, 56, 379, 56, 384, 57, 384, 57, 389, 59, 389,
        59, 394, 60, 394, 60, 398, 62, 398, 62, 403, 63, 403, 63, 407, 65, 407,
        65, 411, 67, 411, 67, 414, 69, 414, 69, 418, 71, 418, 71, 421, 73, 421,
        73, 424, 75, 424, 75, 427, 77, 427, 77, 430, 79, 430, 79, 433, 81, 433,
        81, 435, 84, 435, 84, 438, 86, 438, 86, 441, 88, 441, 88, 445, 90, 445,
        90, 448, 92, 448, 92, 452, 94, 452, 94, 457, 96, 457, 96, 462, 98, 462,
        98, 467, 100, 467, 100, 472, 101, 472, 101, 477, 102, 477, 102, 482,
        102, 482, 102, 487, 102, 487, 102, 491, 101, 491, 101, 496, 99, 496, 99,
        499, 96, 499, 96, 503, 93, 503, 93, 506, 89, 506, 89, 508, 84, 508, 84,
        511, 80, 511, 80, 513, 75, 513, 75, 515, 71, 515, 71, 517, 68, 517, 68,
        519, 65, 519, 65, 522, 64, 522, 64, 519, 73, 519, 73, 517, 80, 517, 80,
        514, 86, 514, 86, 512, 91, 512, 91, 510, 95, 510, 95, 509, 98, 509, 98,
        507, 101, 507, 101, 505, 103, 505, 103, 503, 104, 503, 104, 502, 106,
        502, 106, 500, 107, 500, 107, 498, 108, 498, 108, 497, 109, 497, 109,
        495, 109, 495, 109, 493, 110, 493, 110, 491, 111, 491, 111, 490, 111,
        490, 111, 488, 112, 488, 112, 486, 112, 486, 112, 484, 112, 484, 112,
        483, 113, 483, 113, 481, 113, 481, 113, 479, 113, 479, 113, 477, 113,
        477, 113, 475, 113, 475, 113, 473, 113, 473, 113, 471, 113, 471, 113,
        469, 113, 469, 113, 467, 112, 467, 112, 465, 112, 465, 112, 463, 112,
        463, 112, 461, 111, 461, 111, 459, 111, 459, 111, 457, 110, 457, 110,
        456, 109, 456, 109, 454, 109, 454, 109, 452, 108, 452, 108, 450, 107,
        450, 107, 448, 106, 448, 106, 446, 105, 446, 105, 444, 104, 444, 104,
        442, 103, 442, 103, 440, 102, 440, 102, 438, 101, 438, 101, 436, 100,
        436, 100, 433, 99, 433, 99, 431, 97, 431, 97, 429, 96, 429, 96, 426, 95,
        426, 95, 424, 94, 424, 94, 421, 93, 421, 93, 419, 92, 419, 92, 416, 91,
        416, 91, 413, 89, 413, 89, 410, 88, 410, 88, 407, 87, 407, 87, 403, 86,
        403, 86, 400, 86, 400, 86, 396, 85, 396, 85, 392, 84, 392, 84, 389, 83,
        389, 83, 385, 83, 385, 83, 380, 83, 380, 83, 376, 82, 376, 82, 372, 82,
        372, 82, 367, 82, 367, 82, 363, 83, 363, 83, 358, 83, 358, 83, 353, 84,
        353, 84, 348, 85, 348, 85, 343, 86, 343, 86, 338, 88, 338, 88, 333, 90,
        333, 90, 328, 91, 328, 91, 323, 93, 323, 93, 317, 96, 317, 96, 312, 98,
        312, 98, 307, 100, 307, 100, 301, 102, 301, 102, 296, 104, 296, 104,
        291, 106, 291, 106, 285, 108, 285, 108, 280, 109, 280, 109, 275, 111,
        275, 111, 269, 112, 269, 112, 264, 113, 264, 113, 259, 114, 259, 114,
        253, 115, 253, 115, 248, 115, 248, 115, 243, 116, 243, 116, 237, 116,
        237, 116, 232, 116, 232, 116, 227, 116, 227, 116, 221, 116, 221, 116,
        216, 116, 216, 116, 210, 115, 210, 115, 205, 115, 205, 115, 199, 114,
        199, 114, 194, 113, 194, 113, 188, 112, 188, 112, 182, 111, 182, 111,
        176, 110, 176, 110, 169, 108, 169, 108, 163, 106, 163, 106, 155, 103,
        155, 103, 147, 99, 147, 99, 139, 94, 139, 94, 129, 88, 129, 88, 119, 80,
        119, 80, 108, 70, 108, 70, 95, 58, 0, 55, 1, 55, 1, 55, 2, 56, 2, 56, 4,
        56, 4, 56, 5, 56, 5, 56, 6, 57, 6, 57, 7, 58, 7, 58, 8, 58, 8, 58, 8,
        59, 8, 59, 9, 60, 9, 60, 10, 60, 10, 60, 10, 61, 10, 61, 11, 62, 11, 62,
        11, 63, 11, 63, 11, 64, 11, 64, 12, 64, 12, 65, 12, 66, 12, 66, 12, 67,
        12, 67, 12, 68, 12, 68, 12, 69, 12, 69, 12, 70, 12, 70, 12, 71, 12, 71,
        12, 72, 12, 72, 12, 73, 12, 73, 11, 73,
    };

    uint16_t xoff = 640 / 2 - 522 / 2;
    uint16_t yoff = 480 / 2 - 116 / 2;

    for (size_t i = 0; i < 1090; i += 4) {
        line(xoff + python[i], yoff + python[i + 1], xoff + python[i + 2], yoff + python[i + 3]);
    }
}

void vga_set_videomode(const vga_videomode* mode) {
    VGA_WRITE(VGA_PORT_MISC_WRITE, ((vga_misc) {
        .io_emulation_mode = VGA_EMULATION_MODE_COLOR,
        .enable_vram_access = 1,
        .clock_select = mode->clock_speed,
        .odd_even_select_high_page = false,
        .hsync_polarity = VGA_POLARITY_NEGATIVE,
        .vsync_polarity = VGA_POLARITY_NEGATIVE,
    }));

    blank_and_unlock();
    set_crtc_registers(mode);
    set_grc_registers(mode);
    set_seq_registers(mode);
    set_atc_registers(mode);
    unblank_and_lock();

    reset_cursor();
    clear_vram(VGA_PLANE_ALL);

    VGA_SETTINGS.width_pixels = mode->horizontal_timings.active_area;
    VGA_SETTINGS.height_pixels = mode->vertical_timings.active_area;

    uint8_t h_div = mode->dot_mode == VGA_DOT_MODE_9_DPC ? 9 : 8;
    VGA_SETTINGS.width_chars = mode->horizontal_timings.active_area / h_div;

    draw_python();
}

uint16_t vga_get_width_pixels() {
    return VGA_SETTINGS.width_pixels;
}

uint16_t vga_get_height_pixels() {
    return VGA_SETTINGS.height_pixels;
}

uint8_t vga_get_width_chars() {
    return VGA_SETTINGS.width_chars;
}
