#ifndef _CHEESOS2_INCLUDE_INTERRUPT_REGISTERS_H
#define _CHEESOS2_INCLUDE_INTERRUPT_REGISTERS_H

#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint16_t ss;
    uint16_t reserved_1;
    uint32_t esp;
    uint32_t eflags;
    uint16_t cs;
    uint16_t reserved_2;
    uint32_t eip;
} interrupt_parameters_full;

typedef struct __attribute__((packed)) {
    uint32_t eflags;
    uint16_t cs;
    uint16_t reserved;
    uint32_t eip;
} interrupt_parameters;

typedef struct __attribute__((packed)) {
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
} interrupt_registers;

#endif
