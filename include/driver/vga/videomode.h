#ifndef _CHEESOS2_DRIVER_VGA_VIDEOMODE_H
#define _CHEESOS2_DRIVER_VGA_VIDEOMODE_H

#include <stdint.h>
#include <stdbool.h>

struct vga_crt_timings {
    // Values in pixels/scanlines, regardless of 8 or 9 pixels per character

    uint16_t active_area;
    uint8_t overscan_front;
    uint8_t blanking_front;
    uint8_t retrace;
    uint8_t blanking_back;
    uint8_t overscan_back;
};

typedef struct {
    struct vga_crt_timings horizontal_timings;
    struct vga_crt_timings vertical_timings;
} vga_videomode;

extern const vga_videomode VGA_VIDEOMODE_640x480x16;

void vga_set_videomode(const vga_videomode* mode);

#endif
