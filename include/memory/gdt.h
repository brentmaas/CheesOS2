#ifndef _CHEESOS2_INCLUDE_MEMORY_GDT_H
#define _CHEESOS2_INCLUDE_MEMORY_GDT_H

#include <stdint.h>

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

#endif
