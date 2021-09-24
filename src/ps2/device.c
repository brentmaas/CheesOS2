#include "ps2/device.h"
#include "ps2/controller.h"
#include "core/io.h"
#include "interrupt/idt.h"
#include "interrupt/pic.h"
#include "debug/log.h"

#define PS2_CONTROLLER_PORT (0x64)
#define PS2_DEVICE_PORT (0x60)

#define UNUSED(x) ((void)x)

#define PS2_CONTROLLER_SECOND (0xD4)

enum ps2_device_command_type {
    PS2_DEVICE_COMMAND_NONE = 0,
    PS2_DEVICE_COMMAND_DISABLE_SCAN = 0xF5,
    PS2_DEVICE_COMMAND_IDENTIFY = 0xF2,
    PS2_DEVICE_COMMAND_RESET = 0xFF
};

enum ps2_device_state {
    PS2_STATE_INITIAL,
    PS2_STATE_FAIL,
    PS2_STATE_WAIT_ACK,
    PS2_STATE_ID_WAIT_ACK,
    PS2_STATE_ID_READ
};

enum ps2_device_response {
    PS2_RESPONSE_ACK = 0xFA,
    PS2_RESPONSE_RESEND = 0XFE
};

#define PS2_COMMAND_MAX_SIZE (8)

static volatile enum ps2_device_state state_1;
static volatile uint8_t device_1_last_command[PS2_COMMAND_MAX_SIZE];
static volatile size_t device_1_last_command_size;
static volatile uint8_t device_1_last_data;
static volatile enum ps2_device_state state_2;
static volatile uint8_t device_2_last_command[PS2_COMMAND_MAX_SIZE];
static volatile size_t device_2_last_command_size;
static volatile uint8_t device_2_last_data;

static volatile bool ps2_device_identification_complete;
static volatile enum ps2_device_type ps2_device_identification;

void ps2_device_send(enum ps2_device_id device, volatile uint8_t* command, size_t command_size);

void ps2_device_handle_interrupt(volatile enum ps2_device_state* state) {
    uint8_t data = io_in8(PS2_DEVICE_PORT);
    //log_debug("Current state %u, data: %u (0x%X)", (unsigned)*state, data, data);
    if(state == &state_1) device_1_last_data = data;
    else device_2_last_data = data;
    switch(*state) {
        case PS2_STATE_INITIAL:
            break;
        case PS2_STATE_WAIT_ACK:
            if(data == PS2_RESPONSE_ACK) {
                *state = PS2_STATE_INITIAL;
            } else {
                ps2_device_send(state == &state_1 ? PS2_DEVICE_FIRST : PS2_DEVICE_SECOND, device_1_last_command, device_1_last_command_size);
            }
            break;
        case PS2_STATE_ID_WAIT_ACK:
            if(data == PS2_RESPONSE_ACK) {
                *state = PS2_STATE_ID_READ;
            } else {
                ps2_device_send(state == &state_1 ? PS2_DEVICE_FIRST : PS2_DEVICE_SECOND, device_1_last_command, device_1_last_command_size);
            }
            break;
        case PS2_STATE_ID_READ:
            //log_debug("Read id byte: %x", (unsigned)data);
            if(data != PS2_DEVICE_TYPE_RESPONSE_MF2_KEYBOARD_FIRST){
                *state = PS2_STATE_INITIAL;
                ps2_device_identification_complete = true;
            }
            switch(data){
                case PS2_DEVICE_TYPE_RESPONSE_MOUSE:
                    ps2_device_identification = PS2_DEVICE_TYPE_MOUSE;
                    break;
                case PS2_DEVICE_TYPE_RESPONSE_MOUSE_SCROLL:
                    ps2_device_identification = PS2_DEVICE_TYPE_MOUSE_SCROLL;
                    break;
                case PS2_DEVICE_TYPE_RESPONSE_MOUSE_5_BUTTON:
                    ps2_device_identification = PS2_DEVICE_TYPE_MOUSE_5_BUTTON;
                    break;
                case PS2_DEVICE_TYPE_RESPONSE_MF2_KEYBOARD_TRANSLATION:
                    ps2_device_identification = PS2_DEVICE_TYPE_MF2_KEYBOARD_TRANSLATION;
                    break;
                case PS2_DEVICE_TYPE_RESPONSE_MF2_KEYBOARD:
                    ps2_device_identification = PS2_DEVICE_TYPE_MF2_KEYBOARD;
                    break;
            }
            break;
        case PS2_STATE_FAIL:
            break;
    }
    //log_debug("Transitioned to state %u", (unsigned)*state);
}

void ps2_device_master_interrupt_callback(uint32_t interrupt, struct interrupt_registers* registers, struct interrupt_parameters* params) {
    UNUSED(interrupt);
    UNUSED(registers);
    UNUSED(params);

    //log_debug("Master PS/2 interrupt triggered");
    ps2_device_handle_interrupt(&state_1);

    pic_end_interrupt(PIC_MASTER);
}

void ps2_device_slave_interrupt_callback(uint32_t interrupt, struct interrupt_registers* registers, struct interrupt_parameters* params) {
    UNUSED(interrupt);
    UNUSED(registers);
    UNUSED(params);

    //log_debug("Slave PS/2 interrupt triggered");
    ps2_device_handle_interrupt(&state_2);

    pic_end_interrupt(PIC_SLAVE);
}

void ps2_device_register_interrupts(uint8_t pic_device_1, uint8_t pic_device_2) {
    idt_make_interrupt_no_status(pic_device_1, ps2_device_master_interrupt_callback, IDT_GATE_TYPE_INTERRUPT_32, IDT_FLAG_PRESENT);
    idt_make_interrupt_no_status(pic_device_2, ps2_device_slave_interrupt_callback, IDT_GATE_TYPE_INTERRUPT_32, IDT_FLAG_PRESENT);
}

void ps2_device_send(enum ps2_device_id device, volatile uint8_t* command, size_t command_size) {
    for(size_t i = 0; i < command_size; ++i) {
        if(device == PS2_DEVICE_SECOND) {
            ps2_controller_wait_output();
            io_out8(PS2_CONTROLLER_PORT, PS2_CONTROLLER_SECOND);
        }
        ps2_controller_wait_output();
        io_out8(PS2_DEVICE_PORT, command[i]);
    }
}

void ps2_device_send_command(enum ps2_device_id device, uint8_t command) {
    volatile enum ps2_device_state* state = device == PS2_DEVICE_SECOND ? &state_2 : &state_1;

    switch(command) {
        case PS2_DEVICE_COMMAND_RESET:
        case PS2_DEVICE_COMMAND_DISABLE_SCAN:
            *state = PS2_STATE_WAIT_ACK;
            break;
        case PS2_DEVICE_COMMAND_IDENTIFY:
            *state = PS2_STATE_ID_WAIT_ACK;
            break;
    }

    if(device == PS2_DEVICE_SECOND) {
        device_2_last_command[0] = command;
        device_2_last_command_size = 1;
        ps2_device_send(device, device_2_last_command, device_2_last_command_size);
    } else {
        device_1_last_command[0] = command;
        device_1_last_command_size = 1;
        ps2_device_send(device, device_1_last_command, device_1_last_command_size);
    }
}

void ps2_device_wait_for_response(enum ps2_device_id device) {
    volatile enum ps2_device_state* state = device == PS2_DEVICE_SECOND ? &state_2 : &state_1;

    while(!(*state == PS2_STATE_INITIAL || *state == PS2_STATE_FAIL));
}

void ps2_device_reset(enum ps2_device_id device) {
    //log_debug("Resetting device %u", (unsigned)device);
    ps2_device_send_command(device, PS2_DEVICE_COMMAND_RESET);
    ps2_device_wait_for_response(device);

    //log_debug("End of device reset");
}

enum ps2_device_type ps2_device_identify(enum ps2_device_id device) {
    //log_debug("Identifying device %u", (unsigned)device);
    ps2_device_send_command(device, PS2_DEVICE_COMMAND_DISABLE_SCAN);
    //???
    /*if (device == PS2_DEVICE_SECOND) {
        while (true) {
            while (true) {
                if (io_in8(PS2_CONTROLLER_PORT) & 0x1) {
                    break;
                }
            }

            uint8_t data = io_in8(PS2_DEVICE_PORT);
            log_debug("Device responded with 0x%02X", data);
        }
    }*/

    ps2_device_wait_for_response(device);
    
    //Will likely loop infinitely with an AT keyboard, but works on my machine
    ps2_device_identification = PS2_DEVICE_TYPE_AT_KEYBOARD;
    ps2_device_identification_complete = false;
    
    ps2_device_send_command(device, PS2_DEVICE_COMMAND_IDENTIFY);
    ps2_device_wait_for_response(device);
    
    while(!ps2_device_identification_complete);

    //log_debug("End of device identification");
    
    return ps2_device_identification;
}

uint8_t ps2_device_get_last_data(enum ps2_device_id device){
    return device == PS2_DEVICE_FIRST ? device_1_last_data : device_2_last_data;
}