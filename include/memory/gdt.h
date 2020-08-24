#ifndef _CHEESOS2_INCLUDE_MEMORY_GDT_H
#define _CHEESOS2_INCLUDE_MEMORY_GDT_H

#include <stdint.h>
#include <stdnoreturn.h>

struct __attribute__((packed)) gdt_descriptor {
    uint16_t size;
    void* addr;
};

struct  __attribute__((packed)) gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t limit_high : 4;
    uint8_t flags : 4;
    uint8_t base_high;
};

struct __attribute__((packed)) gdt_tss {
    uint32_t link;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint16_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint16_t debug_trap;
    uint16_t iomap_base;
};

enum gdt_access_type {
    GDT_ACCESS_ACCESSED = 1 << 0,
    GDT_ACCESS_RW = 1 << 1,
    GDT_ACCESS_DIRECT_CONFIRM = 1 << 2,
    GDT_ACCESS_EXECUTE = 1 << 3,
    GDT_ACCESS_SYSTEM = 1 << 4,
    GDT_ACCESS_PRIVILEGE = (1 << 5) | (1 << 6),
    GDT_ACCESS_PRESENT = 1 << 7
};

#define GDT_ACCESS_PRIVILEGE_0 (0u)
#define GDT_ACCESS_PRIVILEGE_1 (1u << 5u)
#define GDT_ACCESS_PRIVILEGE_2 (1u << 6u)
#define GDT_ACCESS_PRIVILEGE_3 ((1u << 5u) | (1u << 6u))

enum gdt_flags_type {
    GDT_FLAG_AVL = 1 << 0,
    GDT_FLAG_LONG_MODE = 1 << 1,
    GDT_FLAG_SIZE = 1 << 2,
    GDT_FLAG_GRANULARITY = 1 << 3
};

void gdt_init(void);

// Set the stack used on the next interrupt
void gdt_set_int_stack(void* new_stack);

extern noreturn void gdt_jump_to_usermode(void* user_code, void* user_stack);

#endif
