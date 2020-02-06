#include "memory/gdt.h"

gdt_entry entries[3];

void gdt_load_entry(gdt_entry* entry, size_t base, size_t limit, gdt_flags_type flags, gdt_access_type access) {
    
}

void gdt_init(void) {
    //Selector 0, has to be 0
    gdt_load_entry(&entries[0], 0, 0, 0, 0);

    //Selector 0x8, code segment kernel level
    gdt_load_entry(&entries[1],
        0,
        0xFFFFFFFFu,
        GDT_FLAG_SIZE | GDT_GRANULARITY,
        GDT_ACCESS_PRESENT | GDT_ACCESS_PRIVILEGE_0 | GDT_ACCESS_SYSTEM | GDT_ACCESS_EXECUTABLE | GDT_ACCESS_RW
    );
}