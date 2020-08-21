#ifndef _CHEESOS2_DRIVER_VGA_IO_H
#define _CHEESOS2_DRIVER_VGA_IO_H

#include <stdint.h>
#include <stdbool.h>

#include "core/io.h"
#include "utility/bitcast.h"

#define VGA_PORT_GRC_ADDR 0x3CE
#define VGA_PORT_GRC_DATA 0x3CF

#define VGA_PORT_SEQ_ADDR 0x3C4
#define VGA_PORT_SEQ_DATA 0x3C5

// This port is used both as address and data: first byte is address, second is data
// Reset the internal flip-flop by reading from input status #1
#define VGA_PORT_ATC 0x3C0
#define VGA_PORT_ATC_READ 0x03C1

#define VGA_PORT_CRTC_MONO_ADDR 0x3B4
#define VGA_PORT_CRTC_MONO_DATA 0x3B5

#define VGA_PORT_CRTC_COLOR_ADDR 0x3D4
#define VGA_PORT_CRTC_COLOR_DATA 0x3D5

#define VGA_PORT_DAC_ADDR_WRITE 0x3C8
#define VGA_PORT_DAC_ADDR_READ 0x3C7
#define VGA_PORT_DAC_DATA 0x03C9
#define VGA_PORT_DAC_STATE 0x3C7

#define VGA_PORT_MISC_WRITE 0x3C2
#define VGA_PORT_MISC_READ 0x3CC

#define VGA_PORT_FEATURE_MONO_READ 0x3CA
#define VGA_PORT_FEATURE_MONO_WRITE 0x3BA

#define VGA_PORT_FEATURE_READ 0x3CA
#define VGA_PORT_FEATURE_WRITE_MONO 0x3BA
#define VGA_PORT_FEATURE_WRITE_COLOR 0x3DA

#define VGA_PORT_INPUT_STATUS_0 0x3C2
#define VGA_PORT_INPUT_STATUS_1_MONO 0x3BA
#define VGA_PORT_INPUT_STATUS_1_COLOR 0x3DA

#define VGA_READ(port, reg_ptr)                                 \
    do {                                                        \
        typedef typeof(*(reg_ptr)) _reg_type;                   \
        *(reg_ptr) = BITCAST(_reg_type, io_in8(port));          \
    } while (0)

#define VGA_WRITE(port, reg) io_out8(port, BITCAST(uint8_t, reg))

void vga_sync_atc();
void vga_prepare_atc(uint8_t index, bool lock_palette);

struct vga_io_state {
    uint8_t grc;
    uint8_t seq;
    uint8_t crtc;
    uint8_t atc;
};

void vga_read_io_state(struct vga_io_state* state);
void vga_write_io_state(const struct vga_io_state* state);

#endif
