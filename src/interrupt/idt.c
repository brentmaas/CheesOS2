#include "interrupt/idt.h"

#include <stdio.h>

static idt_descriptor descriptor;
static idt_entry entries[256];

void* idt_callback_routines[256];

void* idt_hardware_callbacks[256];

extern void idt_create_handler_table(void* table);
extern void idt_load(void* descriptor);

void idt_set_entry(idt_entry* entry, uint32_t offset, uint16_t selector, idt_gate_type gate_type, idt_flag_type flags) {
    entry->offset_low = offset & 0xFFFFu;
    entry->offset_high = (offset & 0xFFFF0000u) >> 16u;

    entry->zero = 0;

    entry->selector = selector;
    entry->gate_type = gate_type;
    entry->flags = flags;
}

void idt_make_interrupt(size_t interrupt, void* callback, idt_gate_type callback_type, idt_flag_type flags) {
    idt_callback_routines[interrupt] = callback;

    idt_set_entry(&entries[interrupt], (uint32_t)idt_hardware_callbacks[interrupt], 0x08, callback_type, flags);
}

void idt_make_interrupt_no_status(size_t interrupt, interrupt_no_status callback, idt_gate_type callback_type, idt_flag_type flags) {
    idt_make_interrupt(interrupt, (void*)callback, callback_type, flags);
}

void idt_make_interrupt_status(size_t interrupt, interrupt_status callback, idt_gate_type callback_type, idt_flag_type flags) {
    idt_make_interrupt(interrupt, (void*)callback, callback_type, flags);
}

void no_status_callback(interrupt_registers* registers, interrupt_parameters* parameter) {
    printf("No status interrupt at address %x\n", parameter->eip);
}

void status_callback(interrupt_registers* registers, interrupt_parameters* parameter, uint32_t status) {
    printf("No status interrupt at address %x, status = %u\n", parameter->eip, status);
}

void double_fault_callback(interrupt_registers* registers, interrupt_parameters* parameter, uint32_t status) {
    printf("Oef: double fault at address %x\n", parameter->eip);
}

void userspace_callback(interrupt_registers* registers, interrupt_parameters* parameter) {
    printf("Software interrupt with EAX=%x\n", registers->eax);
}

void idt_init(void) {
    idt_create_handler_table(idt_hardware_callbacks);

    idt_make_interrupt_no_status(0, no_status_callback, IDT_GATE_TYPE_TRAP_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT);
    idt_make_interrupt(1, NULL, 0, 0); //TODO: Debug
    idt_make_interrupt(2, NULL, 0, 0); //TODO: Non-maskable interrupt
    idt_make_interrupt_no_status(3, no_status_callback, IDT_GATE_TYPE_TRAP_32, IDT_PRIVILEGE_3 | IDT_FLAG_PRESENT);
    idt_make_interrupt_no_status(4, no_status_callback, IDT_GATE_TYPE_TRAP_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT);
    idt_make_interrupt_no_status(5, no_status_callback, IDT_GATE_TYPE_TRAP_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT);
    idt_make_interrupt_no_status(6, no_status_callback, IDT_GATE_TYPE_TRAP_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT);
    idt_make_interrupt_no_status(7, no_status_callback, IDT_GATE_TYPE_TRAP_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT);
    idt_make_interrupt_status(8, double_fault_callback, IDT_GATE_TYPE_TRAP_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT);
    idt_make_interrupt_no_status(9, no_status_callback, IDT_GATE_TYPE_TRAP_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT);
    idt_make_interrupt_status(10, status_callback, IDT_GATE_TYPE_TRAP_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT);
    idt_make_interrupt_status(11, status_callback, IDT_GATE_TYPE_TRAP_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT);
    idt_make_interrupt_status(12, status_callback, IDT_GATE_TYPE_TRAP_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT);
    idt_make_interrupt_status(13, status_callback, IDT_GATE_TYPE_TRAP_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT);
    idt_make_interrupt_status(14, status_callback, IDT_GATE_TYPE_TRAP_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT);
    idt_make_interrupt(15, NULL, 0, 0);
    idt_make_interrupt_no_status(16, no_status_callback, IDT_GATE_TYPE_TRAP_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT);
    idt_make_interrupt_status(17, status_callback, IDT_GATE_TYPE_TRAP_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT);
    idt_make_interrupt(18, no_status_callback, IDT_GATE_TYPE_TRAP_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT);
    idt_make_interrupt(19, NULL, 0, 0);
    idt_make_interrupt(20, NULL, 0, 0);
    idt_make_interrupt_no_status(21, no_status_callback, IDT_GATE_TYPE_TRAP_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT);
    for(int i = 22; i < 32; ++i) {
        idt_make_interrupt(i, NULL, 0, 0);
    }

    for(int i = 32; i < 66; ++i) {
        idt_make_interrupt(i, NULL, 0, 0);
    }

    idt_make_interrupt_no_status(66, userspace_callback, IDT_GATE_TYPE_TRAP_32, IDT_PRIVILEGE_3 | IDT_FLAG_PRESENT);

    for(int i = 67; i < 256; ++i) {
        idt_make_interrupt(i, NULL, 0, 0);
    }

    descriptor.size = sizeof(entries) - 1;
    descriptor.addr = entries;

    idt_load(&descriptor);
}