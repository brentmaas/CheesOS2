#include "ps2/scancode.h"
#include "ps2/controller.h"

//Zie https://wiki.osdev.org/PS/2_Keyboard, hele 0xe0 rommel moet ook ooit nog (tm)
//Voel je vrij betere namen te geven als je wilt
enum ps2_scancode2 {
    PS2_KEY_F9 = 0x1,
    PS2_KEY_F5 = 0x3,
    PS2_KEY_F3 = 0x4,
    PS2_KEY_F1 = 0x5,
    PS2_KEY_F2 = 0x6,
    PS2_KEY_F12 = 0x7,
    PS2_KEY_F10 = 0x9,
    PS2_KEY_F8 = 0xa,
    PS2_KEY_F6 = 0xb,
    PS2_KEY_F4 = 0xc,
    PS2_KEY_TAB = 0xd,
    PS2_KEY_BACK_TICK = 0xe,
    PS2_KEY_LEFT_ALT = 0x11,
    PS2_KEY_LEFT_SHIFT = 0x12,
    PS2_KEY_LEFT_CONTROL = 0x14,
    PS2_KEY_Q = 0x15,
    PS2_KEY_1 = 0x16,
    PS2_KEY_Z = 0x1a,
    PS2_KEY_S = 0x1b,
    PS2_KEY_A = 0x1c,
    PS2_KEY_W = 0x1d,
    PS2_KEY_2 = 0x16,
    PS2_KEY_C = 0x21,
    PS2_KEY_X = 0x22,
    PS2_KEY_D = 0x23,
    PS2_KEY_E = 0x24,
    PS2_KEY_4 = 0x25,
    PS2_KEY_3 = 0x26,
    PS2_KEY_SPACE = 0x29,
    PS2_KEY_V = 0x2a,
    PS2_KEY_F = 0x2b,
    PS2_KEY_T = 0x2c,
    PS2_KEY_R = 0x2d,
    PS2_KEY_5 = 0x2e,
    PS2_KEY_N = 0x31,
    PS2_KEY_B = 0x32,
    PS2_KEY_H = 0x33,
    PS2_KEY_G = 0x34,
    PS2_KEY_Y = 0x35,
    PS2_KEY_6 = 0x36,
    PS2_KEY_M = 0x3a,
    PS2_KEY_J = 0x3b,
    PS2_KEY_U = 0x3c,
    PS2_KEY_7 = 0x3d,
    PS2_KEY_8 = 0x3e,
    PS2_KEY_COMMA = 0x41,
    PS2_KEY_K = 0x42,
    PS2_KEY_I = 0x43,
    PS2_KEY_O = 0x44,
    PS2_KEY_0 = 0x45,
    PS2_KEY_9 = 0x46,
    PS2_KEY_PERIOD = 0x49,
    PS2_KEY_SLASH = 0x4a,
    PS2_KEY_L = 0x4b,
    PS2_KEY_SEMICOLON = 0x4c,
    PS2_KEY_P = 0x4d,
    PS2_KEY_MINUS = 0x4e,
    PS2_KEY_APOSTROPHE = 0x52,
    PS2_KEY_LEFT_SQUARE_BRACKET = 0x54,
    PS2_KEY_EQUALS = 0x55,
    PS2_KEY_CAPSLOCK = 0x58,
    PS2_KEY_RIGHT_SHIFT = 0x59,
    PS2_KEY_ENTER = 0x5a,
    PS2_KEY_RIGHT_SQUARE_BRACKET = 0x5b,
    PS2_KEY_BACKSLASH = 0x5d,
    PS2_KEY_BACKSPACE = 0x66,
    PS2_KEY_NUMPAD_1 = 0x69,
    PS2_KEY_NUMPAD_4 = 0x6b,
    PS2_KEY_NUMPAD_7 = 0x6c,
    PS2_KEY_NUMPAD_0 = 0x70,
    PS2_KEY_NUMPAD_PERIOD = 0x71,
    PS2_KEY_NUMPAD_2 = 0x72,
    PS2_KEY_NUMPAD_5 = 0x73,
    PS2_KEY_NUMPAD_6 = 0x74,
    PS2_KEY_NUMPAD_8 = 0x75,
    PS2_KEY_ESCAPE = 0x76,
    PS2_KEY_NUMLOCK = 0x77,
    PS2_KEY_F11 = 0x78,
    PS2_KEY_NUMPAD_PLUS = 0x79,
    PS2_KEY_NUMPAD_3 = 0x7a,
    PS2_KEY_NUMPAD_MINUS = 0x7b,
    PS2_KEY_NUMPAD_ASTERISK = 0x7c,
    PS2_KEY_NUMPAD_9 = 0x7d,
    PS2_KEY_SCROLLLOCK = 0x7e
};

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

static char ps2_get_char(uint8_t scancode){
    uint8_t is_upper = ((ps2_scancode_set2_caps_affected[scancode] & PS2_CAPSLOCK_ACTIVE) + (PS2_LEFT_SHIFT_DOWN | PS2_RIGHT_SHIFT_DOWN)) % 2;
    if(is_upper) return ps2_scancode_set2_upper[scancode];
    return ps2_scancode_set2_lower[scancode];
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
