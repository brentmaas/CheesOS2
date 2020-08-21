#ifndef _CHEESOS2_INCLUDE_INTERRUPT_REGISTERS_H
#define _CHEESOS2_INCLUDE_INTERRUPT_REGISTERS_H

#include <stdint.h>

struct __attribute__((packed)) interrupt_parameters_full {
    uint32_t eip;
    uint16_t cs;
    uint16_t reserved;
    uint32_t eflags;
    uint32_t esp;
    uint16_t ss;
    uint16_t reserved_2;
};

struct __attribute__((packed)) interrupt_parameters {
    uint32_t eip;
    uint16_t cs;
    uint16_t reserved;
    uint32_t eflags;
};

struct __attribute__((packed)) interrupt_registers {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
};

#endif
