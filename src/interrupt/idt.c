#include "interrupt/idt.h"
#include "vga/vga.h"
#include "debug/memdump.h"
#include "interrupt/exceptions.h"

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

void idt_init(void) {
    idt_create_handler_table(idt_hardware_callbacks);

    for(size_t i = 0; i < 256; ++i) {
        idt_make_interrupt_no_status(i, NULL, IDT_GATE_TYPE_TRAP_32, IDT_PRIVILEGE_0);
    }

    idt_exceptions_load();

    descriptor.size = sizeof(entries) - 1;
    descriptor.addr = entries;

    idt_load(&descriptor);
}