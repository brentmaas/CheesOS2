#include "ps2/keyboard.h"

#include "core/io.h"

#include "interrupt/idt.h"
#include "interrupt/pic.h"

#include "utility/containers/ringbuffer.h"

#include "debug/log.h"

#define PS2_CONTROLLER_PORT (0x64)
#define PS2_DEVICE_PORT (0x60)

#define PS2_KEYBOARD_COMMAND_MAX_SIZE (8)

enum ps2_keyboard_command_type {
    PS2_KEYBOARD_COMMAND_LEDS = 0xED,
    PS2_KEYBOARD_COMMAND_ECHO = 0xEE,
    PS2_KEYBOARD_COMMAND_SCANCODE = 0xF0,
    PS2_KEYBOARD_COMMAND_IDENTIFY = 0xF2,
    PS2_KEYBOARD_COMMAND_TYPEMATIC = 0xF3,
    PS2_KEYBOARD_COMMAND_ENABLE_SCAN = 0xF4,
    PS2_KEYBOARD_COMMAND_DISABLE_SCAN = 0xF5,
    PS2_KEYBOARD_COMMAND_DEFAULT = 0xF6,
    PS2_KEYBOARD_COMMAND_TYPEMATIC_AUTOREPEAT = 0xF7,
    PS2_KEYBOARD_COMMAND_MAKE_RELEASE = 0xF8,
    PS2_KEYBOARD_COMMAND_MAKE = 0xF9,
    PS2_KEYBOARD_COMMAND_TYPEMATIC_AUTOREPEAT_MAKE_RELEASE = 0xFA,
    PS2_KEYBOARD_COMMAND_TYPEMATIC_AUTOREPEAT_KEY = 0xFB,
    PS2_KEYBOARD_COMMAND_MAKE_RELEASE_KEY = 0xFC,
    PS2_KEYBOARD_COMMAND_MAKE_KEY = 0xFD,
    PS2_KEYBOARD_COMMAND_RESEND = 0xFE,
    PS2_KEYBOARD_COMMAND_SELFTEST = 0xFF
    
};

enum ps2_keyboard_state {
    PS2_KEYBOARD_STATE_INITIAL
};

static uint8_t scancodeset2[512] = {
    //0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
    //lower, ralt off
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, '`',   0,
      0,   0,   0,   0,   0, 'q', '1',   0,   0,   0, 'z', 's', 'a', 'w', '2',   0,
      0, 'c', 'x', 'd', 'e', '4', '3',   0,   0, ' ', 'v', 'f', 't', 'r', '5',   0,
      0, 'n', 'b', 'h', 'g', 'y', '6',   0,   0,   0, 'm', 'j', 'u', '7', '8',   0,
      0, ',', 'k', 'i', 'o', '0', '9',   0,   0, '.', '/', 'l', ';', 'p', '-',   0,
      0,   0,'\'',   0, '[', '=',   0,   0,   0,   0,'\n', ']',   0,'\\',   0,   0,
      0,   0,   0,   0,   0,   0,   8,   0,   0, '1',   0, '4', '7',   0,   0,   0,
    '0', '.', '2', '5', '6', '8',  27,   0,   0, '+', '3', '-', '*', '9',   0,   0,
    //lower, ralt on
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, '`',   0,
      0,   0,   0,   0,   0, 'q', '1',   0,   0,   0, 'z', 's', 'a', 'w', '2',   0,
      0, 'c', 'x', 'd', 'e', '4', '3',   0,   0, ' ', 'v', 'f', 't', 'r', '5',   0,
      0, 'n', 'b', 'h', 'g', 'y', '6',   0,   0,   0, 'm', 'j', 'u', '7', '8',   0,
      0, ',', 'k', 'i', 'o', '0', '9',   0,   0, '.', '/', 'l', ';', 'p', '-',   0,
      0,   0,'\'',   0, '[', '=',   0,   0,   0,   0,'\n', ']',   0,'\\',   0,   0,
      0,   0,   0,   0,   0,   0,   8,   0,   0, '1',   0, '4', '7',   0,   0,   0,
    '0', '.', '2', '5', '6', '8',  27,   0,   0, '+', '3', '-', '*', '9',   0,   0,
    //upper, ralt off
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, '~',   0,
      0,   0,   0,   0,   0, 'Q', '!',   0,   0,   0, 'Z', 'S', 'A', 'W', '@',   0,
      0, 'C', 'X', 'D', 'E', '$', '#',   0,   0, ' ', 'V', 'F', 'T', 'R', '%',   0,
      0, 'N', 'B', 'H', 'G', 'Y', '^',   0,   0,   0, 'M', 'J', 'U', '&', '*',   0,
      0, '<', 'K', 'I', 'O', ')', '(',   0,   0, '>', '?', 'L', ':', 'P', '_',   0,
      0,   0,'\"',   0, '{', '+',   0,   0,   0,   0,'\n', '}',   0, '|',   0,   0,
      0,   0,   0,   0,   0,   0,   8,   0,   0, '1',   0, '4', '7',   0,   0,   0,
    '0', '.', '2', '5', '6', '8',  27,   0,   0, '+', '3', '-', '*', '9',   0,   0,
    //upper, ralt on
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, '~',   0,
      0,   0,   0,   0,   0, 'Q', '!',   0,   0,   0, 'Z', 'S', 'A', 'W', '@',   0,
      0, 'C', 'X', 'D', 'E', '$', '#',   0,   0, ' ', 'V', 'F', 'T', 'R', '%',   0,
      0, 'N', 'B', 'H', 'G', 'Y', '^',   0,   0,   0, 'M', 'J', 'U', '&', '*',   0,
      0, '<', 'K', 'I', 'O', ')', '(',   0,   0, '>', '?', 'L', ':', 'P', '_',   0,
      0,   0,'\"',   0, '{', '+',   0,   0,   0,   0,'\n', '}',   0, '|',   0,   0,
      0,   0,   0,   0,   0,   0,   8,   0,   0, '1',   0, '4', '7',   0,   0,   0,
    '0', '.', '2', '5', '6', '8',  27,   0,   0, '+', '3', '-', '*', '9',   0,   0
};

static volatile struct {
    bool numberlock;
    bool capslock;
    bool scrolllock;
    bool lshift;
    bool rshift;
    bool lctrl;
    bool rctrl;
    bool lalt;
    bool ralt;
} ps2_keyboard_modifiers = {false, false, false, false, false, false, false};

static volatile enum ps2_keyboard_state state_1;
static volatile uint8_t keyboard_1_last_command[PS2_KEYBOARD_COMMAND_MAX_SIZE];
static volatile uint8_t keyboard_1_last_command_size;
static volatile enum ps2_keyboard_state state_2;
static volatile uint8_t keyboard_2_last_command[PS2_KEYBOARD_COMMAND_MAX_SIZE];
static volatile uint8_t keyboard_2_last_command_size;

static volatile ringbuffer ps2_keyboard_buffer;
static volatile bool ps2_keyboard_buffer_initialised = false;

void ps2_keyboard_handle_interrupt(volatile enum ps2_keyboard_state* state){
    uint8_t data = io_in8(PS2_DEVICE_PORT);
    switch(*state){
        case PS2_KEYBOARD_STATE_INITIAL:
            ringbuffer_put(&ps2_keyboard_buffer, data);
            break;
    }
    //log_debug("Keyboard: %u (0x%X), length: %u", data, data, ringbuffer_length(&ps2_keyboard_buffer));
}

void ps2_keyboard_master_interrupt_callback(uint32_t interrupt, struct interrupt_registers* registers, struct interrupt_parameters* params){
    ps2_keyboard_handle_interrupt(&state_1);
    
    pic_end_interrupt(PIC_MASTER);
}

void ps2_keyboard_slave_interrupt_callback(uint32_t interrupt, struct interrupt_registers* registers, struct interrupt_parameters* params){
    ps2_keyboard_handle_interrupt(&state_2);
    
    pic_end_interrupt(PIC_SLAVE);
}

void ps2_keyboard_init(enum ps2_device_id device){
    ps2_device_send_command(device, PS2_KEYBOARD_COMMAND_DEFAULT);
    ps2_device_wait_for_response(device);
    
    ps2_device_send_command(device, PS2_KEYBOARD_COMMAND_SCANCODE);
    ps2_device_wait_for_response(device);
    ps2_device_send_command(device, 0);
    ps2_device_wait_for_response(device);
    uint8_t scancode = ps2_device_get_last_data(device);
    //log_debug("Keyboard: default scancode set: %u", scancode);
    
    if(scancode != 2){
        //log_debug("Keyboard: requesting scancode set 2");
        ps2_device_send_command(device, PS2_KEYBOARD_COMMAND_SCANCODE);
        ps2_device_wait_for_response(device);
        ps2_device_send_command(device, 2);
        ps2_device_wait_for_response(device);
    }
    
    ps2_device_send_command(device, PS2_KEYBOARD_COMMAND_ENABLE_SCAN);
    ps2_device_wait_for_response(device);
    
    if(!ps2_keyboard_buffer_initialised) ringbuffer_init(&ps2_keyboard_buffer);
    
    if(device == PS2_DEVICE_FIRST) idt_make_interrupt_no_status(0x21, ps2_keyboard_master_interrupt_callback, IDT_GATE_TYPE_INTERRUPT_32, IDT_FLAG_PRESENT);
    else idt_make_interrupt_no_status(0x2C, ps2_keyboard_slave_interrupt_callback, IDT_GATE_TYPE_INTERRUPT_32, IDT_FLAG_PRESENT);
}

void ps2_keyboard_get_next(uint8_t* next, bool* is_release){
    *is_release = false;
    
    bool done = false;
    while(!done){
        done = true;
        while(!ringbuffer_length(&ps2_keyboard_buffer));
        ringbuffer_read(&ps2_keyboard_buffer, next, 1);
        log_debug("%u (0x%X)", *next, *next);
        if(*next == 0xF0){ //Release
            *is_release = true;
            done = false;
        }else if (*next == 0x11){ //lalt
            ps2_keyboard_modifiers.lalt = !*is_release;
        }else if (*next == 0x12){ //lshift
            ps2_keyboard_modifiers.lshift = !*is_release;
        }else if (*next == 0x14){ //lctrl
            ps2_keyboard_modifiers.lctrl = !*is_release;
        }else if (*next == 0x58){ //caps
            ps2_keyboard_modifiers.capslock = !*is_release;
        }else if (*next == 0x59){ //rshift
            ps2_keyboard_modifiers.rshift = !*is_release;
        }else if (*next == 0x77){ //numberlock
            ps2_keyboard_modifiers.numberlock = !*is_release;
        }else if (*next == 0x7E){ //scrolllock
            ps2_keyboard_modifiers.scrolllock = !*is_release;
        }
    }
}

void ps2_keyboard_get_next_char(uint8_t* next, bool* is_release){
    *is_release = false;
    
    bool done = false;
    while(!done){
        while(!ringbuffer_length(&ps2_keyboard_buffer));
        ringbuffer_read(&ps2_keyboard_buffer, next, 1);
        if(*next == 0xF0){ //Release
            *is_release = true;
        }else if (*next == 0x11){ //lalt
            ps2_keyboard_modifiers.lalt = !*is_release;
            *is_release = false;
        }else if (*next == 0x12){ //lshift
            ps2_keyboard_modifiers.lshift = !*is_release;
            *is_release = false;
        }else if (*next == 0x14){ //lctrl
            ps2_keyboard_modifiers.lctrl = !*is_release;
            *is_release = false;
        }else if (*next == 0x58){ //caps
            ps2_keyboard_modifiers.capslock = !*is_release;
            *is_release = false;
        }else if (*next == 0x59){ //rshift
            ps2_keyboard_modifiers.rshift = !*is_release;
            *is_release = false;
        }else if (*next == 0x77){ //numberlock
            ps2_keyboard_modifiers.numberlock = !*is_release;
            *is_release = false;
        }else if (*next == 0x7E){ //scrolllock
            ps2_keyboard_modifiers.scrolllock = !*is_release;
            *is_release = false;
        }else{
            done = true;
        }
    }
    
    *next = scancodeset2[((size_t) *next) + (ps2_keyboard_modifiers.capslock ^ (ps2_keyboard_modifiers.lshift | ps2_keyboard_modifiers.rshift) ? 256 : 0) + (ps2_keyboard_modifiers.ralt ? 128 : 0)];
}