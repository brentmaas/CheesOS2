#ifndef _CHEESOS2_DRIVER_VGA_VIDEOMODE_H
#define _CHEESOS2_DRIVER_VGA_VIDEOMODE_H

#include <stdint.h>
#include <stdbool.h>
#include "driver/vga/registers.h"

#define VGA_DEFAULT_CHARACTER_HEIGHT (16)

struct vga_crt_timings {
    // Values in pixels/scanlines, regardless of 8 or 9 dots per character
    // These fields are in chronological order

    uint16_t active_area;
    uint8_t overscan_back; // right or bottom
    uint8_t blanking_back;
    uint8_t retrace;
    uint8_t blanking_front; // left or top
    uint8_t overscan_front;
};

enum vga_mode {
    VGA_MODE_TEXT,
    VGA_MODE_GRAPHICS_2_COLOR,
    VGA_MODE_GRAPHICS_4_COLOR,
    VGA_MODE_GRAPHICS_16_COLOR,
    VGA_MODE_GRAPHICS_256_COLOR
};

// Return the amount of bits required to store a value for a particular color depth
#define VGA_COLOR_DEPTH_REQUIRED_BITS(color_depth) (1 << (color_depth))

enum vga_address_mode {
    VGA_ADDRESS_MODE_BYTES,
    VGA_ADDRESS_MODE_WORDS,
    VGA_ADDRESS_MODE_DWORDS
};

// Return size in bytes of element corresponding to an address mode
#define VGA_ADDRESS_MODE_SIZE(address_mode) (1 << (address_mode))

struct vga_videomode {
    struct vga_crt_timings horizontal_timings;
    struct vga_crt_timings vertical_timings;

    enum vga_dot_mode dot_mode : 1;
    enum vga_clock_speed clock_speed : 2;
};

extern const struct vga_videomode VGA_VIDEOMODE_640x480;

void vga_set_videomode(const struct vga_videomode* vidmode, enum vga_mode mode);

uint16_t vga_get_width_pixels();
uint16_t vga_get_height_pixels();
uint8_t vga_get_width_chars();

#endif
