#ifndef _CHEESOS2_INCLUDE_MEMORY_GDT_H
#define _CHEESOS2_INCLUDE_MEMORY_GDT_H

#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint16_t size;
    void* addr;
} gdt_descriptor;

typedef struct  __attribute__((packed)) {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t limit_high : 4;
    uint8_t flags : 4;
    uint8_t base_high;
} gdt_entry;

typedef enum {
    GDT_ACCESS_ACCESSED = 1,
    GDT_ACCESS_RW = 2,
    GDT_ACCESS_DIRECT_CONFIRM = 4,
    GDT_ACCESS_EXECUTE = 8,
    GDT_ACCESS_SYSTEM = 16,
    GDT_ACCESS_PRIVILEGE = 32 | 64,
    GDT_ACCESS_PRIVILEGE_0 = 0,
    GDT_ACCESS_PRIVILEGE_1 = 32,
    GDT_ACCESS_PRIVILEGE_2 = 64,
    GDT_ACCESS_PRIVILEGE_3 = 32 | 64,
    GDT_ACCESS_PRESENT = 128
} gdt_access_type;

typedef enum {
    GDT_FLAG_SIZE = 2,
    GDT_FLAG_GRANULARITY = 4
} gdt_flags_type;

void gdt_init(void);

#endif
