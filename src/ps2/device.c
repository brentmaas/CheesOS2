#include "ps2/device.h"
#include "core/io.h"
#include "interrupt/idt.h"
#include "interrupt/pic.h"

#define PS2_CONTROLLER_PORT (0x64)
#define PS2_DEVICE_PORT (0x60)

#define UNUSED(x) ((void)x)

enum ps2_device_command_type {
    PS2_DEVICE_COMMAND_DISABLE_SCAN = 0xF5,
    PS2_DEVICE_COMMAND_IDENTIFY = 0xFA
};

void ps2_device_master_interrupt_callback(uint32_t interrupt, struct interrupt_registers* registers, struct interrupt_parameters* params) {
    UNUSED(interrupt);
    UNUSED(registers);
    UNUSED(params);

    uint8_t data = io_in8(PS2_DEVICE_PORT);

    pic_end_interrupt(PIC_MASTER);
}

void ps2_device_slave_interrupt_callback(uint32_t interrupt, struct interrupt_registers* registers, struct interrupt_parameters* params) {
    UNUSED(interrupt);
    UNUSED(registers);
    UNUSED(params);

    uint8_t data = io_in8(PS2_DEVICE_PORT);

    pic_end_interrupt(PIC_SLAVE);
}

void ps2_device_register_interrupts(uint8_t pic_device_1, uint8_t pic_device_2) {
    idt_make_interrupt_no_status(pic_device_1, ps2_device_master_interrupt_callback, IDT_GATE_TYPE_INTERRUPT_32, IDT_FLAG_PRESENT);
    idt_make_interrupt_no_status(pic_device_2, ps2_device_slave_interrupt_callback, IDT_GATE_TYPE_INTERRUPT_32, IDT_FLAG_PRESENT);
}

enum ps2_device_type ps2_device_identify(enum ps2_device_id device) {

}