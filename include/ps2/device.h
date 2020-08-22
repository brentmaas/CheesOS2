#ifndef _CHEESOS2_PS2_DEVICE_H
#define _CHEESOS2_PS2_DEVICE_H

#include <stdint.h>

enum ps2_device_id {
    PS2_DEVICE_FIRST,
    PS2_DEVICE_SECOND
};

enum ps2_device_type {
    PS2_DEVICE_TYPE_DISABLED,
    PS2_DEVICE_TYPE_AT_KEYBOARD,
    PS2_DEVICE_TYPE_MOUSE,
    PS2_DEVICE_TYPE_MOUSE_SCROLL,
    PS2_DEVICE_TYPE_MOUSE_5_BUTTON,
    PS2_DEVICE_TYPE_MF2_KEYBOARD_TRANSLATION,
    PS2_DEVICE_TYPE_MF2_KEYBOARD
};

void ps2_device_register_interrupts(uint8_t, uint8_t);
enum ps2_device_type ps2_device_identify(enum ps2_device_id);

#endif
