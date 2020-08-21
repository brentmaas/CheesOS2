#include "driver/vga/util.h"
#include "driver/vga/io.h"

bool vga_enable_odd_even(bool enabled) {
    io_out8(VGA_PORT_SEQ_ADDR, VGA_IND_SEQ_MEMORY_MODE);
    struct vga_seq_memory_mode seq_mode;
    VGA_READ(VGA_PORT_SEQ_DATA, &seq_mode);
    bool was_enabled = !seq_mode.disable_odd_even;
    seq_mode.disable_odd_even = !enabled;
    VGA_WRITE(VGA_PORT_SEQ_DATA, seq_mode);

    io_out8(VGA_PORT_GRC_ADDR, VGA_IND_GRC_MODE);
    struct vga_grc_mode grc_mode;
    VGA_READ(VGA_PORT_GRC_DATA, &grc_mode);
    grc_mode.enable_host_odd_even = enabled;
    VGA_WRITE(VGA_PORT_GRC_DATA, grc_mode);
    return was_enabled;
}

enum vga_plane_bits vga_mask_planes(enum vga_plane_bits planes) {
    io_out8(VGA_PORT_SEQ_ADDR, VGA_IND_SEQ_MAP_MASK);
    struct vga_seq_map_mask mask;
    VGA_READ(VGA_PORT_SEQ_DATA, &mask);

    VGA_WRITE(VGA_PORT_SEQ_DATA, ((struct vga_seq_map_mask) {
        .planes = planes,
    }));

    return mask.planes;
}

enum vga_memory_map vga_map_memory(enum vga_memory_map map) {
    io_out8(VGA_PORT_GRC_ADDR, VGA_IND_GRC_MISC);
    struct vga_grc_misc grc_misc;
    VGA_READ(VGA_PORT_GRC_DATA, &grc_misc);
    enum vga_memory_map orig_map = grc_misc.memory_map;
    grc_misc.memory_map = map;
    VGA_WRITE(VGA_PORT_GRC_DATA, grc_misc);
    return orig_map;
}
