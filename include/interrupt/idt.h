#ifndef _CHEESOS2_INCLUDE_INTERRUPT_IDT_H
#define _CHEESOS2_INCLUDE_INTERRUPT_IDT_H

#include <stdint.h>
#include <stddef.h>

#include "interrupt/registers.h"

struct __attribute__((packed)) idt_descriptor {
    uint16_t size;
    void* addr;
};

struct __attribute__((packed)) idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t gate_type : 4;
    uint8_t flags : 4;
    uint16_t offset_high;
};

enum idt_gate_type {
    IDT_GATE_TYPE_TASK_32 = 0x5,
    IDT_GATE_TYPE_INTERRUPT_16 = 0x6,
    IDT_GATE_TYPE_TRAP_16 = 0x7,
    IDT_GATE_TYPE_INTERRUPT_32 = 0xE,
    IDT_GATE_TYPE_TRAP_32 = 0xF
};

enum idt_flag_type {
    IDT_FLAG_STORAGE = 1 << 0,
    IDT_FLAG_IOPL = (1 << 1) | (1 << 2),
    IDT_FLAG_PRESENT = 1 << 3
};

#define IDT_PRIVILEGE_0 (0u)
#define IDT_PRIVILEGE_1 (1u << 1u)
#define IDT_PRIVILEGE_2 (1u << 2u)
#define IDT_PRIVILEGE_3 ((1u << 1u) | (1u << 2u))

typedef void(*interrupt_no_status)(uint32_t, struct interrupt_registers*, struct interrupt_parameters*);
typedef void(*interrupt_status)(uint32_t, struct interrupt_registers*, struct interrupt_parameters*, uint32_t);

void idt_init(void);
void idt_enable(void);
void idt_disable(void);

//NOTE TO ROBIN: DO NOT CALL UNLESS INTERRUPTS ARE DISABLED
void idt_make_interrupt_no_status(size_t interrupt, interrupt_no_status callback, enum idt_gate_type callback_type, enum idt_flag_type flags);
void idt_make_interrupt_status(size_t interrupt, interrupt_status callback, enum idt_gate_type callback_type, enum idt_flag_type flags);

#endif
