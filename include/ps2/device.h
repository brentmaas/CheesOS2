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

enum ps2_device_type_response {
    PS2_DEVICE_TYPE_RESPONSE_MOUSE = 0x00,
    PS2_DEVICE_TYPE_RESPONSE_MOUSE_SCROLL = 0x03,
    PS2_DEVICE_TYPE_RESPONSE_MOUSE_5_BUTTON = 0x04,
    PS2_DEVICE_TYPE_RESPONSE_MF2_KEYBOARD_FIRST = 0xAB,
    PS2_DEVICE_TYPE_RESPONSE_MF2_KEYBOARD_TRANSLATION = 0x41,
    PS2_DEVICE_TYPE_RESPONSE_MF2_KEYBOARD_TRANSLATION2 = 0xC1,
    PS2_DEVICE_TYPE_RESPONSE_MF2_KEYBOARD = 0x83
};

void ps2_device_register_interrupts(uint8_t pic_device_1, uint8_t pic_device_2);
void ps2_device_send_command(enum ps2_device_id device, uint8_t command);
void ps2_device_wait_for_response(enum ps2_device_id device);
void ps2_device_reset(enum ps2_device_id device);
enum ps2_device_type ps2_device_identify(enum ps2_device_id device);
uint8_t ps2_device_get_last_data(enum ps2_device_id device);

#endif
