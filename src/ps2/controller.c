#include "ps2/controller.h"
#include "ps2/device.h"
#include "ps2/keyboard.h"

#include "core/io.h"
#include "debug/log.h"

#define PS2_COMMAND_PORT (0x64)
#define PS2_DATA_PORT (0x60)

enum ps2_status_type {
    PS2_STATUS_OUTPUT = 0x1,
    PS2_STATUS_INPUT = 0x2,
    PS2_STATUS_SYSTEM = 0x4,
    PS2_STATUS_COMMAND = 0x8,
    PS2_STATUS_UNKNOWN = 0x10,
    PS2_STATUS_UNKNOWN2 = 0x20,
    PS2_STATUS_TIMEOUT = 0x40,
    PS2_STATUS_PARITY = 0x80
};

enum ps2_config_type {
    PS2_CONFIG_FIRST_INTERRUPT = 0x1,
    PS2_CONFIG_SECOND_INTERRUPT = 0x2,
    PS2_CONFIG_SYSTEM = 0x4,
    PS2_CONFIG_ZERO = 0x8,
    PS2_CONFIG_FIRST_CLOCK = 0x10,
    PS2_CONFIG_SECOND_CLOCK = 0x20,
    PS2_CONFIG_FIRST_TRANSLATE = 0x40,
    PS2_CONFIG_ZERO2 = 0x80
};

enum ps2_command_type {
    PS2_COMMAND_READ_CONFIG = 0x20,
    PS2_COMMAND_WRITE_CONFIG = 0x60,
    PS2_COMMAND_DISABLE_SECOND = 0xA7,
    PS2_COMMAND_ENABLE_SECOND = 0xA8,
    PS2_COMMAND_TEST_SECOND = 0xA9,
    PS2_COMMAND_SELF_TEST = 0xAA,
    PS2_COMMAND_TEST_FIRST = 0xAB,
    PS2_COMMAND_DISABLE_FIRST = 0xAD,
    PS2_COMMAND_ENABLE_FIRST = 0xAE
};

enum ps2_response_type {
    PS2_RESPONSE_SELF_TEST_OK = 0x55
};

enum ps2_device_type ps2_port1_device = PS2_DEVICE_TYPE_DISABLED;
enum ps2_device_type ps2_port2_device = PS2_DEVICE_TYPE_DISABLED;

bool ps2_controller_has_input(void) {
    return io_in8(PS2_COMMAND_PORT) & PS2_STATUS_OUTPUT;
}

void ps2_controller_wait_output(void) {
    while(io_in8(PS2_COMMAND_PORT) & PS2_STATUS_INPUT); // Input buffer status bit must be 0
}

void ps2_controller_wait_input(void) {
    while(!ps2_controller_has_input()); // Output buffer status bit must be 1
}

void ps2_clear_output(void) {
    while(io_in8(PS2_COMMAND_PORT) & PS2_STATUS_OUTPUT) {
        ((void)io_in8(PS2_DATA_PORT));
    }
}

void ps2_controller_send_buffer(uint8_t command, const uint8_t* data, size_t data_size) {
    ps2_controller_wait_output();
    io_out8(PS2_COMMAND_PORT, command);

    for(size_t i = 0; i < data_size; ++i) {
        ps2_controller_wait_output();
        io_out8(PS2_DATA_PORT, data[i]);
    }
}

void ps2_controller_send(uint8_t command) {
    ps2_controller_send_buffer(command, NULL, 0);
}

void ps2_controller_send_byte(uint8_t command, uint8_t data) {
    ps2_controller_send_buffer(command, &data, 1);
}

void ps2_controller_read_data(uint8_t* data, size_t bytes) {
    for(size_t i = 0; i < bytes; ++i) {
        ps2_controller_wait_input();
        data[i] = io_in8(PS2_DATA_PORT);
    }
}

uint8_t ps2_controller_read(void) {
    uint8_t result;
    ps2_controller_read_data(&result, 1);
    return result;
}

uint8_t ps2_read_config_byte(void) {
    ps2_controller_send(PS2_COMMAND_READ_CONFIG);
    return ps2_controller_read();
}

void ps2_write_config_byte(uint8_t config) {
    ps2_controller_send_byte(PS2_COMMAND_WRITE_CONFIG, config);
}

int ps2_controller_init(void) {
    ps2_controller_send(PS2_COMMAND_DISABLE_FIRST);
    ps2_controller_send(PS2_COMMAND_DISABLE_SECOND);

    ps2_clear_output();

    uint8_t ps2_config_byte = ps2_read_config_byte();
    //log_debug("PS/2 config byte: %X", (unsigned)ps2_config_byte);
    bool is_dual_port = ps2_config_byte & PS2_CONFIG_SECOND_CLOCK;
    //log_debug("Dual port: %d", (int)is_dual_port);
    ps2_config_byte &= ~(PS2_CONFIG_FIRST_INTERRUPT | PS2_CONFIG_SECOND_INTERRUPT | PS2_CONFIG_FIRST_TRANSLATE);
    //log_debug("Writing config byte %X", (unsigned)ps2_config_byte);
    ps2_write_config_byte(ps2_config_byte);

    ps2_controller_send(PS2_COMMAND_SELF_TEST);
    uint8_t response = ps2_controller_read();
    //log_debug("Self test resonse: %X", (unsigned)response);
    if(response != PS2_RESPONSE_SELF_TEST_OK)
        return -1;
    ps2_write_config_byte(ps2_config_byte);

    if(is_dual_port) {
        ps2_controller_send(PS2_COMMAND_ENABLE_SECOND);
        uint8_t ps2_config_byte2 = ps2_read_config_byte();
        //log_debug("Config byte after second controller: %X", (unsigned)ps2_config_byte2);

        is_dual_port = !(ps2_config_byte2 & PS2_CONFIG_SECOND_CLOCK);

        if(is_dual_port) {
            ps2_controller_send(PS2_COMMAND_DISABLE_SECOND);
        }
    }

    ps2_controller_send(PS2_COMMAND_TEST_FIRST);
    bool first_good = !ps2_controller_read();
    //log_debug("First device test: %d", (int)first_good);

    bool second_good = false;
    if(is_dual_port) {
        ps2_controller_send(PS2_COMMAND_TEST_SECOND);
        second_good = !ps2_controller_read();
    }
    //log_debug("Second device test: %d", (int)second_good);

    if(first_good || second_good) {
        if(first_good)
            ps2_config_byte |= PS2_CONFIG_FIRST_INTERRUPT;
        if(second_good)
            ps2_config_byte |= PS2_CONFIG_SECOND_INTERRUPT;
        //log_debug("Writing config byte: %X", ps2_config_byte);
        ps2_write_config_byte(ps2_config_byte);
    }

    if(first_good)
        ps2_controller_send(PS2_COMMAND_ENABLE_FIRST);
    if(second_good)
        ps2_controller_send(PS2_COMMAND_ENABLE_SECOND);

    //log_debug("Initializing first controller");

    if(first_good) {
        ps2_device_reset(PS2_DEVICE_FIRST);
        ps2_port1_device = ps2_device_identify(PS2_DEVICE_FIRST);
    }
    
    //log_debug("First device: %u (0x%X)", ps2_port1_device, ps2_port1_device);

    //log_debug("Initializing second controller");

    if(second_good) {
        ps2_device_reset(PS2_DEVICE_SECOND);
        ps2_port2_device = ps2_device_identify(PS2_DEVICE_SECOND);
    }
    
    //log_debug("Second device: %u (0x%X)", ps2_port2_device, ps2_port2_device);
    
    if(ps2_port1_device == PS2_DEVICE_TYPE_AT_KEYBOARD || ps2_port1_device == PS2_DEVICE_TYPE_MF2_KEYBOARD_TRANSLATION || ps2_port1_device == PS2_DEVICE_TYPE_MF2_KEYBOARD){
        //log_debug("Initializing first device as a keyboard");
        ps2_keyboard_init(PS2_DEVICE_FIRST);
    }
    
    if(ps2_port2_device == PS2_DEVICE_TYPE_AT_KEYBOARD || ps2_port2_device == PS2_DEVICE_TYPE_MF2_KEYBOARD_TRANSLATION || ps2_port2_device == PS2_DEVICE_TYPE_MF2_KEYBOARD){
        //log_debug("Initializing second device as a keyboard");
        ps2_keyboard_init(PS2_DEVICE_SECOND);
    }

    return 0;
}
