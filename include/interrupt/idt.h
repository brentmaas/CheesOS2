#ifndef _CHEESOS2_INCLUDE_INTERRUPT_IDT_H
#define _CHEESOS2_INCLUDE_INTERRUPT_IDT_H

#include <stdint.h>
#include <stddef.h>

#include "interrupt/registers.h"

typedef struct __attribute__((packed)) {
    uint16_t size;
    void* addr;
} idt_descriptor;

typedef struct __attribute__((packed)) {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t gate_type : 4;
    uint8_t flags : 4;
    uint16_t offset_high;
} idt_entry;

typedef enum {
    IDT_GATE_TYPE_TASK_32 = 0x5,
    IDT_GATE_TYPE_INTERRUPT_16 = 0x6,
    IDT_GATE_TYPE_TRAP_16 = 0x7,
    IDT_GATE_TYPE_INTERRUPT_32 = 0xE,
    IDT_GATE_TYPE_TRAP_32 = 0xF
} idt_gate_type;

typedef enum {
    IDT_FLAG_STORAGE = 1 << 0,
    IDT_FLAG_IOPL = (1 << 1) | (1 << 2),
    IDT_FLAG_PRESENT = 1 << 4
} idt_flag_type;

#define IDT_PRIVILEGE_0 (0u)
#define IDT_PRIVILEGE_1 (1u << 1u)
#define IDT_PRIVILEGE_2 (1u << 2u)
#define IDT_PRIVILEGE_3 ((1u << 1u) | (1u << 2u))

typedef void(*interrupt_no_status)(interrupt_registers*, interrupt_parameters*);
typedef void(*interrupt_status)(interrupt_registers*, interrupt_parameters*, uint32_t);

void idt_init(void);
void idt_enable(void);
void idt_disable(void);


void idt_make_interrupt(size_t interrupt, void* callback, idt_gate_type callback_type, idt_flag_type flags);
void idt_make_interrupt_no_status(size_t interrupt, interrupt_no_status callback, idt_gate_type callback_type, idt_flag_type flags);
void idt_make_interrupt_status(size_t interrupt, interrupt_status callback, idt_gate_type callback_type, idt_flag_type flags);

#endif
