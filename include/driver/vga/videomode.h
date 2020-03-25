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

typedef enum {
    VGA_MODE_TEXT,
    VGA_MODE_GRAPHICS_2_COLOR,
    VGA_MODE_GRAPHICS_4_COLOR,
    VGA_MODE_GRAPHICS_16_COLOR,
    VGA_MODE_GRAPHICS_256_COLOR
} vga_mode;

// Return the amount of bits required to store a value for a particular color depth
#define VGA_COLOR_DEPTH_REQUIRED_BITS(color_depth) (1 << (color_depth))

typedef enum {
    VGA_ADDRESS_MODE_BYTES,
    VGA_ADDRESS_MODE_WORDS,
    VGA_ADDRESS_MODE_DWORDS
} vga_address_mode;

// Return size in bytes of element corresponding to an address mode
#define VGA_ADDRESS_MODE_SIZE(address_mode) (1 << (address_mode))

typedef struct {
    struct vga_crt_timings horizontal_timings;
    struct vga_crt_timings vertical_timings;

    vga_dot_mode dot_mode : 1;
    vga_clock_speed clock_speed : 2;
} vga_videomode;

extern const vga_videomode VGA_VIDEOMODE_640x480;

void vga_set_videomode(const vga_videomode* vidmode, vga_mode mode);

uint16_t vga_get_width_pixels();
uint16_t vga_get_height_pixels();
uint8_t vga_get_width_chars();

#endif