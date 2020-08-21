#ifndef _CHEESOS2_DRIVER_VGA_UTIL_H
#define _CHEESOS2_DRIVER_VGA_UTIL_H

#include "driver/vga/registers.h"
#include <stdbool.h>

// These functions return the previous state of their respective setting
bool vga_enable_odd_even(bool enabled);
enum vga_plane_bits vga_mask_planes(enum vga_plane_bits planes);
enum vga_memory_map vga_map_memory(enum vga_memory_map map);

#endif
