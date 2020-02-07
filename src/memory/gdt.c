#include "memory/gdt.h"

#include "vga/vga.h"
#include "debug/memdump.h"

extern void gdt_load(void*);

static gdt_descriptor __attribute__((section("data"))) descriptor;
static gdt_entry __attribute__((section("data"))) entries[5];

void gdt_load_entry(gdt_entry* entry, uint32_t base, uint32_t limit, gdt_flags_type flags, gdt_access_type access) {
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
        0xFFFFFFFFu,
        GDT_FLAG_SIZE | GDT_FLAG_GRANULARITY,
        GDT_ACCESS_PRESENT | GDT_ACCESS_PRIVILEGE_0 | GDT_ACCESS_SYSTEM | GDT_ACCESS_EXECUTE | GDT_ACCESS_RW
    );

    //Selector 0x10, data segment kernel level
    gdt_load_entry(&entries[2],
        0,
        0xFFFFFFFFu,
        GDT_FLAG_SIZE | GDT_FLAG_GRANULARITY,
        GDT_ACCESS_PRESENT | GDT_ACCESS_PRIVILEGE_0 | GDT_ACCESS_SYSTEM | GDT_ACCESS_RW
    );

    //Selector 0x18, code segment user level
    gdt_load_entry(&entries[3],
        0,
        0xFFFFFFFFu,
        GDT_FLAG_SIZE | GDT_FLAG_GRANULARITY,
        GDT_ACCESS_PRESENT | GDT_ACCESS_PRIVILEGE_3 | GDT_ACCESS_SYSTEM | GDT_ACCESS_EXECUTE | GDT_ACCESS_RW
    );

    //Selector 0x20, code segment user level
    gdt_load_entry(&entries[4],
        0,
        0xFFFFFFFFu,
        GDT_FLAG_SIZE | GDT_FLAG_GRANULARITY,
        GDT_ACCESS_PRESENT | GDT_ACCESS_PRIVILEGE_3 | GDT_ACCESS_SYSTEM | GDT_ACCESS_RW
    );

    descriptor.size = sizeof(entries);
    descriptor.addr = entries;
    vga_print("Table:\n");
    debug_memdump(&entries[0], sizeof(entries));

    vga_print("Descriptor:\n");
    debug_memdump(&descriptor, sizeof(descriptor));

    asm("cli\nhlt");

    gdt_load(&descriptor);
}