#include "driver/vga/videomode.h"
#include "driver/vga/io.h"
#include "driver/vga/util.h"
#include "core/io.h"
#include "vga/vga.h"

const struct vga_videomode VGA_VIDEOMODE_640x480 = {
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
};

static struct {
    uint16_t width_pixels;
    uint16_t height_pixels;
    uint8_t width_chars;
} VGA_SETTINGS = {};

// In text modes, planes 0 and 1 are used in odd/even mode
// Graphics modes use planar mode
const enum vga_plane_bits DEFAULT_ENABLED_PLANES[] = {
    [VGA_MODE_TEXT] = VGA_PLANE_0_BIT | VGA_PLANE_1_BIT,
    [VGA_MODE_GRAPHICS_2_COLOR] = VGA_PLANE_0_BIT,
    [VGA_MODE_GRAPHICS_4_COLOR] = VGA_PLANE_0_BIT | VGA_PLANE_1_BIT,
    [VGA_MODE_GRAPHICS_16_COLOR] = VGA_PLANE_ALL,
    [VGA_MODE_GRAPHICS_256_COLOR] = VGA_PLANE_ALL,
};

static void dump_registers() {
    vga_printf("misc:\n    %02X\nsequencer:\n    ", io_in8(VGA_PORT_MISC_READ));
    for (size_t i = 0; i < VGA_NUM_SEQ_INDICES; ++i) {
        io_out8(VGA_PORT_SEQ_ADDR, i);
        vga_printf("%02X ", io_in8(VGA_PORT_SEQ_DATA));
    }

    vga_print("\ncrtc:");
    for (size_t i = 0; i < VGA_NUM_CRTC_INDICES; ++i) {
        if (i % 8 == 0) vga_printf("\n    ");
        io_out8(VGA_PORT_CRTC_COLOR_ADDR, i);
        vga_printf("%02X ", io_in8(VGA_PORT_CRTC_COLOR_DATA));
    }

    vga_print("\ngraphics:");
    for (size_t i = 0; i < VGA_NUM_GRC_INDICES; ++i) {
        if (i % 8 == 0) vga_printf("\n    ");
        io_out8(VGA_PORT_GRC_ADDR, i);
        vga_printf("%02X ", io_in8(VGA_PORT_GRC_DATA));
    }

    vga_print("\natc:");
    for (size_t i = 0; i < VGA_NUM_ATC_INDICES; ++i) {
        if (i % 8 == 0) vga_print("\n    ");
        vga_sync_atc();
        VGA_WRITE(VGA_PORT_ATC, ((struct vga_atc_address) {
            .attribute_address = i,
            .lock_palette = true
        }));
        vga_printf("%02X ", io_in8(VGA_PORT_ATC_READ));
    }
}

static void blank_and_unlock() {
    struct vga_crtc_horiz_blanking_end h_blanking_end;
    io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_HORIZ_BLANKING_END);
    VGA_READ(VGA_PORT_CRTC_COLOR_DATA, &h_blanking_end);
    h_blanking_end.enable_vert_retrace_access = true;
    VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, h_blanking_end);

    struct vga_crtc_vert_retrace_end v_retrace_end;
    io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_VERT_RETRACE_END);
    VGA_READ(VGA_PORT_CRTC_COLOR_DATA, &v_retrace_end);
    v_retrace_end.protect = false;
    VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, v_retrace_end);
}

static void unblank_and_lock() {
    struct vga_crtc_vert_retrace_end v_retrace_end;
    io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_VERT_RETRACE_END);
    VGA_READ(VGA_PORT_CRTC_COLOR_DATA, &v_retrace_end);
    v_retrace_end.protect = true;
    VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, v_retrace_end);

    vga_prepare_atc(0, true);
}

static uint8_t pitch(const struct vga_videomode* vidmode, enum vga_mode mode, enum vga_address_mode address_mode) {
    const uint8_t bits_per_plane = 8;
    uint8_t planes;
    uint8_t bits_per_pixel;

    switch (mode) {
        case VGA_MODE_TEXT: {
            // The value of the offset field is multiplied by `2 * memory_address_size`
            // The width in bytes of a column is `columns * memory_address_size`
            // this yields `columns * memory_address_size / (2 * memory_address_size) =
            // columns / 2`.
            uint8_t h_div = vidmode->dot_mode == VGA_DOT_MODE_9_DPC ? 9 : 8;
            uint8_t columns = vidmode->horizontal_timings.active_area / h_div;
            return columns / 2;
        }
        case VGA_MODE_GRAPHICS_2_COLOR:
            planes = 1;
            bits_per_pixel = 1;
            break;
        case VGA_MODE_GRAPHICS_4_COLOR:
            planes = 2;
            bits_per_pixel = 2;
            break;
        case VGA_MODE_GRAPHICS_16_COLOR:
            planes = 4;
            bits_per_pixel = 4;
            break;
        case VGA_MODE_GRAPHICS_256_COLOR:
            planes = 4;
            bits_per_pixel = 8;
            break;
    }

    uint8_t memory_address_size = VGA_ADDRESS_MODE_SIZE(address_mode);

    // According to FreeVGA equation
    uint8_t pixels_per_address = bits_per_plane * planes / bits_per_pixel;
    return vidmode->horizontal_timings.active_area / (pixels_per_address * memory_address_size * 2);
}

// Assumes CRT registers are unlocked (and leaves them unlocked).
static void set_crtc_registers(const struct vga_videomode* vidmode, enum vga_mode mode) {
    uint8_t h_div = vidmode->dot_mode == VGA_DOT_MODE_9_DPC ? 9 : 8;

    // Set horizontal values
    {
        uint8_t h_display_end = vidmode->horizontal_timings.active_area / h_div - 1;
        uint8_t h_blanking_start = h_display_end + vidmode->horizontal_timings.overscan_back / h_div;
        uint8_t h_retrace_start = h_blanking_start + vidmode->horizontal_timings.blanking_back / h_div;
        uint8_t h_retrace_end = h_retrace_start + vidmode->horizontal_timings.retrace / h_div;
        uint8_t h_blanking_end = h_retrace_end + vidmode->horizontal_timings.blanking_front / h_div;
        uint8_t h_total = h_blanking_end + vidmode->horizontal_timings.overscan_front / h_div + 1 - 5;

        io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_HORIZ_TOTAL);
        VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, h_total);

        io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_HORIZ_DISPLAY_END);
        VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, h_display_end);

        io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_HORIZ_BLANKING_START);
        VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, h_blanking_start);

        io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_HORIZ_BLANKING_END);
        VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, ((struct vga_crtc_horiz_blanking_end) {
             // lower 5 bits stored here, 6th bit stored in the retrace end register
            .horiz_blanking_end_low = h_blanking_end & 0x1F,
            .enable_display_skew = false,
            .enable_vert_retrace_access = true
        }));

        io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_HORIZ_RETRACE_START);
        VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, h_retrace_start);

        io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_HORIZ_RETRACE_END);
        VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, ((struct vga_crtc_horiz_retrace_end) {
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
        // blanking" signal becomes active' minus 1.
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

        uint16_t v_display_end = vidmode->vertical_timings.active_area;
        uint16_t v_blanking_start = v_display_end + vidmode->vertical_timings.overscan_back;
        uint16_t v_retrace_start = v_blanking_start + vidmode->vertical_timings.blanking_back;
        uint16_t v_retrace_end = v_retrace_start + vidmode->vertical_timings.retrace;
        uint16_t v_blanking_end = v_retrace_end + vidmode->vertical_timings.blanking_front;
        uint16_t v_total = v_blanking_end + vidmode->vertical_timings.overscan_front;
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
        VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, ((struct vga_crtc_overflow) {
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
        VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, ((struct vga_crtc_maximum_scan_line) {
            .maximum_scan_line = mode == VGA_MODE_TEXT ? VGA_DEFAULT_CHARACTER_HEIGHT : 0,
            .vert_blanking_start_9 = (v_blanking_start >> 9) & 0x1,
            .line_compare_9 = (line_compare >> 9) & 0x1,
            .enable_double_scan = 0
        }));

        io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_VERT_RETRACE_START);
        VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, (uint8_t) v_retrace_start);

        io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_VERT_RETRACE_END);
        VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, ((struct vga_crtc_vert_retrace_end) {
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

    enum vga_address_mode address_mode = mode == VGA_MODE_TEXT ?
        VGA_ADDRESS_MODE_WORDS : VGA_ADDRESS_MODE_BYTES;

    // Set the other display-generation related values
    io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_MODE);
    VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, ((struct vga_crtc_mode) {
        .map13 = true,
        .map14 = true,
        .enable_half_scanline_clock = false,
        .enable_half_memory_clock = false,
        .address_wrap_select = 1,
        .disable_word_addressing = address_mode != VGA_ADDRESS_MODE_WORDS,
        .enable_sync = true
    }));

    io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_OFFSET);
    VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, pitch(vidmode, mode, address_mode));

    io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_UNDERLINE_LOCATION);
    VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, ((struct vga_crtc_underline_location) {
        .underline_location = 0,
        .enable_quarter_memory_clock = false,
        .enable_dword_addressing = address_mode == VGA_ADDRESS_MODE_DWORDS,
    }));

    // Finally set the other CRTC registers
    io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_PRESET_ROW_SCAN);
    VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, ((struct vga_crtc_preset_row_scan) {
        .preset_row_scan = 0,
        .byte_panning = 0
    }));

    io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_START_ADDRESS_HIGH);
    VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, (uint8_t) 0);
    io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_START_ADDRESS_LOW);
    VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, (uint8_t) 0);
}

static void set_grc_registers(enum vga_mode mode) {
    io_out8(VGA_PORT_GRC_ADDR, VGA_IND_GRC_SET_RESET);
    VGA_WRITE(VGA_PORT_GRC_DATA, ((struct vga_grc_set_reset) {
        .planes = 0
    }));

    io_out8(VGA_PORT_GRC_ADDR, VGA_IND_GRC_ENABLE_SET_RESET);
    VGA_WRITE(VGA_PORT_GRC_DATA, ((struct vga_grc_enable_set_reset) {
        .planes = 0
    }));

    io_out8(VGA_PORT_GRC_ADDR, VGA_IND_GRC_COLOR_COMPARE);
    VGA_WRITE(VGA_PORT_GRC_DATA, ((struct vga_grc_color_compare) {
        .planes = 0
    }));

    io_out8(VGA_PORT_GRC_ADDR, VGA_IND_GRC_DATA_ROTATE);
    VGA_WRITE(VGA_PORT_GRC_DATA, ((struct vga_grc_data_rotate) {
        .rotate_count = 0,
        .operation = VGA_OP_NOP
    }));

    io_out8(VGA_PORT_GRC_ADDR, VGA_IND_GRC_READ_MAP);
    VGA_WRITE(VGA_PORT_GRC_DATA, ((struct vga_grc_read_map) {
        .plane = 0
    }));

    io_out8(VGA_PORT_GRC_ADDR, VGA_IND_GRC_MODE);
    VGA_WRITE(VGA_PORT_GRC_DATA, ((struct vga_grc_mode) {
        .write_mode = 0,
        .read_mode = 0,
        .enable_host_odd_even = mode == VGA_MODE_TEXT, // Odd-even mode for reading
        .enable_shift_interleave = false,
        .enable_shift256 = false
    }));

    io_out8(VGA_PORT_GRC_ADDR, VGA_IND_GRC_MISC);
    VGA_WRITE(VGA_PORT_GRC_DATA, ((struct vga_grc_misc) {
        .enable_graphics_mode = mode != VGA_MODE_TEXT,
        .enable_chain_odd_even = false,
        .memory_map = mode == VGA_MODE_TEXT ?
            VGA_MEMORY_MAP_B8000_32K : VGA_MEMORY_MAP_A0000_64K
    }));

    io_out8(VGA_PORT_GRC_ADDR, VGA_IND_GRC_COLOR_DONT_CARE);
    VGA_WRITE(VGA_PORT_GRC_DATA, ((struct vga_grc_color_compare) {
        .planes = VGA_PLANE_ALL
    }));

    io_out8(VGA_PORT_GRC_ADDR, VGA_IND_GRC_BIT_MASK);
    VGA_WRITE(VGA_PORT_GRC_DATA, (uint8_t) 0xFF);
}

static void set_seq_registers(const struct vga_videomode* vidmode, enum vga_mode mode) {
    io_out8(VGA_PORT_SEQ_ADDR, VGA_IND_SEQ_RESET);
    VGA_WRITE(VGA_PORT_SEQ_DATA, ((struct vga_seq_reset) {
        .synchronous_reset = true,
        .asynchronous_reset = true
    }));

    io_out8(VGA_PORT_SEQ_ADDR, VGA_IND_SEQ_CLOCKING_MODE);
    VGA_WRITE(VGA_PORT_SEQ_DATA, ((struct vga_seq_clocking_mode) {
        .dot_mode = vidmode->dot_mode,
        .shift_load_rate = 0,
        .enable_half_dot_clock = false, // TODO: set for 320 and 360 resolutions
        .enable_shift_4 = false,
        .disable_display = false
    }));

    vga_mask_planes(DEFAULT_ENABLED_PLANES[mode]);

    io_out8(VGA_PORT_SEQ_ADDR, VGA_IND_SEQ_CHARACTER_MAP_SELECT);
    VGA_WRITE(VGA_IND_SEQ_CHARACTER_MAP_SELECT, (uint8_t) 0);

    io_out8(VGA_PORT_SEQ_ADDR, VGA_IND_SEQ_MEMORY_MODE);
    VGA_WRITE(VGA_PORT_SEQ_DATA, ((struct vga_seq_memory_mode) {
        .extended_memory = true,
        .disable_odd_even = mode != VGA_MODE_TEXT,
        .enable_chain_4 = false
    }));
}

// Assumes display is blanked and unlocked.
static void set_atc_registers(enum vga_mode mode) {
    // TODO: Allow more control
    for (uint8_t i = 0; i < 16; ++i) {
        vga_prepare_atc(i, false);
        VGA_WRITE(VGA_PORT_ATC, i);
    }

    vga_prepare_atc(VGA_IND_ATC_MODE_CONTROL, false);
    VGA_WRITE(VGA_PORT_ATC, ((struct vga_atc_mode_control) {
        .enable_graphics = mode != VGA_MODE_TEXT,
        .enable_monochrome_emulation = false,
        .enable_line_graphics = true,
        .enable_blink = false,
        .enable_bottom_pixel_pan = false,
        .enable_256_color = mode == VGA_MODE_GRAPHICS_256_COLOR,
        .enable_p45_color_select = false
    }));

    vga_sync_atc();
    VGA_WRITE(VGA_PORT_ATC, ((struct vga_atc_address) {
        .attribute_address = VGA_IND_ATC_OVERSCAN_COLOR,
        .lock_palette = true
    }));

    vga_prepare_atc(VGA_IND_ATC_OVERSCAN_COLOR, false);
    VGA_WRITE(VGA_PORT_ATC, (uint8_t) 0);

    vga_prepare_atc(VGA_IND_ATC_COLOR_PLANE_ENABLE, false);
    VGA_WRITE(VGA_PORT_ATC, ((struct vga_atc_color_plane_enable) {
        .planes = DEFAULT_ENABLED_PLANES[mode]
    }));

    vga_prepare_atc(VGA_IND_ATC_HORIZ_PIXEL_PANNING, false);
    VGA_WRITE(VGA_PORT_ATC, ((struct vga_atc_horiz_pixel_panning) {
        .pixel_shift = 0
    }));

    vga_prepare_atc(VGA_IND_ATC_COLOR_SELECT, false);
    VGA_WRITE(VGA_PORT_ATC, ((struct vga_atc_color_select) {
        .color_select_5_4 = 0,
        .color_select_7_6 = 0
    }));
}

static void reset_cursor() {
    io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_CURSOR_START);
    VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, ((struct vga_crtc_cursor_start) {
        .cursor_scan_line_start = 0,
        .disable_cursor = true
    }));

    io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_CURSOR_END);
    VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, ((struct vga_crtc_cursor_end) {
        .cursor_scan_line_end = VGA_DEFAULT_CHARACTER_HEIGHT - 1,
        .cursor_skew = 0
    }));

    io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_CURSOR_LOCATION_HIGH);
    VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, (uint8_t) 0);
    io_out8(VGA_PORT_CRTC_COLOR_ADDR, VGA_IND_CRTC_CURSOR_LOCATION_LOW);
    VGA_WRITE(VGA_PORT_CRTC_COLOR_DATA, (uint8_t) 0);
}

static void clear_vram(uint8_t planes) {
    const enum vga_memory_map map = VGA_MEMORY_MAP_A0000_64K;
    enum vga_plane_bits orig_mask = vga_mask_planes(planes);
    enum vga_memory_map orig_map = vga_map_memory(map);

    volatile uint8_t* vram = vga_memory_map_ptr(map);
    for (size_t i = 0; i < (1 << 16); ++i) {
        vram[i] = 0;
    }

    vga_map_memory(orig_map);
    vga_mask_planes(orig_mask);
}

void vga_set_videomode(const struct vga_videomode* vidmode, enum vga_mode mode) {
    VGA_WRITE(VGA_PORT_MISC_WRITE, ((struct vga_misc) {
        .io_emulation_mode = VGA_EMULATION_MODE_COLOR,
        .enable_vram_access = 1,
        .clock_select = vidmode->clock_speed,
        .odd_even_select_high_page = false,
        .hsync_polarity = VGA_POLARITY_NEGATIVE,
        .vsync_polarity = VGA_POLARITY_NEGATIVE,
    }));

    blank_and_unlock();
    set_crtc_registers(vidmode, mode);
    set_grc_registers(mode);
    set_seq_registers(vidmode, mode);
    set_atc_registers(mode);
    unblank_and_lock();

    reset_cursor();
    clear_vram(VGA_PLANE_ALL);

    VGA_SETTINGS.width_pixels = vidmode->horizontal_timings.active_area;
    VGA_SETTINGS.height_pixels = vidmode->vertical_timings.active_area;

    uint8_t h_div = vidmode->dot_mode == VGA_DOT_MODE_9_DPC ? 9 : 8;
    VGA_SETTINGS.width_chars = vidmode->horizontal_timings.active_area / h_div;
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
