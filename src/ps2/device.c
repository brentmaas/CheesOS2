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
    PS2_DEVICE_COMMAND_DISABLE_SCAN = 0xF5,
    PS2_DEVICE_COMMAND_IDENTIFY = 0xFA,
    PS2_DEVICE_COMMAND_RESET = 0xFF
};

void ps2_device_master_interrupt_callback(uint32_t interrupt, struct interrupt_registers* registers, struct interrupt_parameters* params) {
    UNUSED(interrupt);
    UNUSED(registers);
    UNUSED(params);

    uint8_t data = io_in8(PS2_DEVICE_PORT);
    log_debug("Master PS/2 interrupt triggered, data 0x%x", (unsigned)data);

    pic_end_interrupt(PIC_MASTER);
}

void ps2_device_slave_interrupt_callback(uint32_t interrupt, struct interrupt_registers* registers, struct interrupt_parameters* params) {
    UNUSED(interrupt);
    UNUSED(registers);
    UNUSED(params);

    uint8_t data = io_in8(PS2_DEVICE_PORT);
    log_debug("Slave PS/2 interrupt triggered, data 0x%x", (unsigned)data);

    pic_end_interrupt(PIC_SLAVE);
}

void ps2_device_register_interrupts(uint8_t pic_device_1, uint8_t pic_device_2) {
    idt_make_interrupt_no_status(pic_device_1, ps2_device_master_interrupt_callback, IDT_GATE_TYPE_INTERRUPT_32, IDT_FLAG_PRESENT);
    idt_make_interrupt_no_status(pic_device_2, ps2_device_slave_interrupt_callback, IDT_GATE_TYPE_INTERRUPT_32, IDT_FLAG_PRESENT);
}

void ps2_device_send(enum ps2_device_id device, uint8_t command) {
    if(device == PS2_DEVICE_SECOND) {
        io_out8(PS2_CONTROLLER_PORT, PS2_CONTROLLER_SECOND);
    }
    ps2_controller_wait_output();
    io_out8(PS2_DEVICE_PORT, command);
}

void ps2_device_reset(enum ps2_device_id device) {
    log_debug("Resetting device %u", (unsigned)device);
    ps2_device_send(device, PS2_DEVICE_COMMAND_RESET);
}

enum ps2_device_type ps2_device_identify(enum ps2_device_id device) {
    log_debug("Identifying device %u", (unsigned)device);
    ps2_device_send(device, PS2_DEVICE_COMMAND_DISABLE_SCAN);
}