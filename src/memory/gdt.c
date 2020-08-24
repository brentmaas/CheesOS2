#include "memory/gdt.h"

extern void gdt_load(void*);
extern void gdt_load_task_register(uint16_t segment);

static struct gdt_descriptor descriptor;
static struct gdt_entry entries[6];

// TODO: Make sure this doesn't get placed over a page boundary
static struct gdt_tss tss = {
    .ss0 = 0x10,
    .esp0 = 0,

    // Intel manual, vol 1, 19.5.2:
    // If the I/O bit map base address is greater than or equal to the TSS segment limit,
    // there is no I/O permission map, and all I/O instructions generate exceptions when
    // the CPL is greater than the current IOPL.
    .iomap_base = sizeof(struct gdt_tss), // disable bitmap
};

void gdt_load_entry(struct gdt_entry* entry, uint32_t base, uint32_t limit, enum gdt_flags_type flags, enum gdt_access_type access) {
    entry->base_low = base & 0xFFFFu;
    entry->base_mid = (base & 0xFF0000u) >> 16u;
    entry->base_high = (base & 0xFF000000u) >> 24u;

    entry->limit_low = limit & 0xFFFFu;
    entry->limit_high = (limit & 0xF0000u) >> 16u;

    entry->access = access;
    entry->flags = flags;
}

void gdt_init(void) {
    //Selector 0, has to be 0
    gdt_load_entry(&entries[0], 0, 0, 0, 0);

    //Selector 0x8, code segment kernel level
    gdt_load_entry(&entries[1],
        0,
        0xFFFFFu,
        GDT_FLAG_SIZE | GDT_FLAG_GRANULARITY,
        GDT_ACCESS_PRESENT | GDT_ACCESS_PRIVILEGE_0 | GDT_ACCESS_SYSTEM | GDT_ACCESS_EXECUTE | GDT_ACCESS_RW
    );

    //Selector 0x10, data segment kernel level
    gdt_load_entry(&entries[2],
        0,
        0xFFFFFu,
        GDT_FLAG_SIZE | GDT_FLAG_GRANULARITY,
        GDT_ACCESS_PRESENT | GDT_ACCESS_PRIVILEGE_0 | GDT_ACCESS_SYSTEM | GDT_ACCESS_RW
    );

    //Selector 0x18, code segment user level
    gdt_load_entry(&entries[3],
        0,
        0xFFFFFu,
        GDT_FLAG_SIZE | GDT_FLAG_GRANULARITY,
        GDT_ACCESS_PRESENT | GDT_ACCESS_PRIVILEGE_3 | GDT_ACCESS_SYSTEM | GDT_ACCESS_EXECUTE | GDT_ACCESS_RW
    );

    //Selector 0x20, data segment user level
    gdt_load_entry(&entries[4],
        0,
        0xFFFFFu,
        GDT_FLAG_SIZE | GDT_FLAG_GRANULARITY,
        GDT_ACCESS_PRESENT | GDT_ACCESS_PRIVILEGE_3 | GDT_ACCESS_SYSTEM | GDT_ACCESS_RW
    );

    //Selector 0x28, kernel TSS segment
    gdt_load_entry(&entries[5],
        (uint32_t) &tss,
        sizeof(struct gdt_tss),
        0,
        GDT_ACCESS_PRESENT | GDT_ACCESS_PRIVILEGE_0 | GDT_ACCESS_EXECUTE | GDT_ACCESS_ACCESSED
    );

    descriptor.size = sizeof(entries) - 1;
    descriptor.addr = entries;

    gdt_load(&descriptor);

    gdt_load_task_register(0x28);
}

void gdt_set_int_stack(void* new_stack) {
    tss.esp0 = (uintptr_t) new_stack;
}
