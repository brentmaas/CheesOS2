#include "ps2/scancode.h"
#include "ps2/controller.h"

const char ps2_scancode_set2_lower[128] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '\t', '`', 0,
    0, 0, 0, 0, 0, 'q', '1', 0, 0, 0, 'z', 's', 'a', 'w', '2', 0,
    0, 'c', 'x', 'd', 'e', '4', '3', 0, 0, ' ', 'v', 'f', 't', 'r', '5', 0,
    0, 'n', 'b', 'h', 'g', 'y', '6', 0, 0, 0, 'm', 'j', 'u', '7', '8', 0,
    0, ',', 'k', 'i', 'o', '0', '9', 0, 0, '.', '/', 'l', ';', 'p', '-', 0,
    0, 0, '\'', 0, '[', '=', 0, 0, 0, 0, '\n', ']', 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, '1', 0, '4', '7', 0, 0, 0,
    '0', '.', '2', '5', '6', '8', 0, 0, 0, '+', '3', '-', '*', '9', 0, 0
};

const char ps2_scancode_set2_upper[128] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '\t', '~', 0,
    0, 0, 0, 0, 0, 'Q', '!', 0, 0, 0, 'Z', 'S', 'A', 'W', '@', 0,
    0, 'C', 'X', 'D', 'E', '$', '#', 0, 0, ' ', 'V', 'F', 'T', 'R', '%', 0,
    0, 'N', 'B', 'H', 'G', 'Y', '^', 0, 0, 0, 'M', 'J', 'U', '&', '*', 0,
    0, '<', 'K', 'I', 'O', ')', '(', 0, 0, '>', '?', 'L', ':', 'P', '_', 0,
    0, 0, '|', 0, '{', '+', 0, 0, 0, 0, '\n', '}', 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, '1', 0, '4', '7', 0, 0, 0,
    '0', '.', '2', '5', '6', '8', 0, 0, 0, '+', '3', '-', '*', '9', 0, 0
};

const uint8_t ps2_scancode_set2_caps_affected[128] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0,
    0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0,
    0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0,
    0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const uint8_t PS2_RELEASE = 0xf0;

static uint8_t PS2_LEFT_SHIFT_DOWN = 0;
static uint8_t PS2_RIGHT_SHIFT_DOWN = 0;
static uint8_t PS2_CAPSLOCK_ACTIVE = 0;
static uint8_t capstoggle = 1;

static void ps2_check_modifier(uint8_t scancode, uint8_t press){
    if(scancode == PS2_KEY_LEFT_SHIFT){
        PS2_LEFT_SHIFT_DOWN = press;
    }else if(scancode == PS2_KEY_RIGHT_SHIFT){
        PS2_RIGHT_SHIFT_DOWN = press;
    }else if(scancode == PS2_KEY_CAPSLOCK && capstoggle){
        PS2_CAPSLOCK_ACTIVE = 1 - PS2_CAPSLOCK_ACTIVE;
        capstoggle = 0;
    }else if(scancode == PS2_KEY_CAPSLOCK && !press){
        capstoggle = 1;
    }
}

char ps2_get_char(uint8_t scancode){
    uint8_t is_upper = ((ps2_scancode_set2_caps_affected[scancode] & PS2_CAPSLOCK_ACTIVE) + (PS2_LEFT_SHIFT_DOWN | PS2_RIGHT_SHIFT_DOWN)) % 2;
    if(is_upper) return ps2_scancode_set2_upper[scancode];
    return ps2_scancode_set2_lower[scancode];
}

uint8_t ps2_read_on_press(){
    uint8_t out = 0;
    while(!out){
        ps2_controller_wait_input();
        uint8_t scancode = ps2_controller_read();
        ps2_check_modifier(scancode, 1);
        if(scancode != PS2_RELEASE) out = scancode;
        else{
            ps2_controller_wait_input();
            ps2_check_modifier(ps2_controller_read(), 0);
        }
    }
    return out;
}

uint8_t ps2_read_on_release(){
    uint8_t out = 0;
    while(!out){
        ps2_controller_wait_input();
        uint8_t scancode = ps2_controller_read();
        ps2_check_modifier(scancode, 1);
        if(scancode == PS2_RELEASE){
            ps2_controller_wait_input();
            scancode = ps2_controller_read();
            ps2_check_modifier(scancode, 0);
            out = scancode;
        }
    }
    return out;
}

char ps2_read_char_on_press(){
    char out = 0;
    while(!out){
        ps2_controller_wait_input();
        uint8_t scancode = ps2_controller_read();
        ps2_check_modifier(scancode, 1);
        if(scancode != PS2_RELEASE) out = ps2_get_char(scancode);
        else{
            ps2_controller_wait_input();
            ps2_check_modifier(ps2_controller_read(), 0);
        }
    }
    return out;
}

char ps2_read_char_on_release(){
    char out = 0;
    while(!out){
        ps2_controller_wait_input();
        uint8_t scancode = ps2_controller_read();
        ps2_check_modifier(scancode, 1);
        if(scancode == PS2_RELEASE){
            ps2_controller_wait_input();
            scancode = ps2_controller_read();
            ps2_check_modifier(scancode, 0);
            out = ps2_get_char(scancode);
        }
    }
    return out;
}
