#ifndef _CHEESOS2_PS2_KEYBOARD_H
#define _CHEESOS2_PS2_KEYBOARD_H

#include <stdbool.h>

#include "ps2/device.h"

void ps2_keyboard_init(enum ps2_device_id device);
void ps2_keyboard_get_next(uint8_t* next, bool* is_release);
void ps2_keyboard_get_next_char(uint8_t* next, bool* is_release);

#endif
