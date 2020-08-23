#include "memory/memory.h"
#include "memory/paging.h"
#include "debug/log.h"

#include <stdint.h>

extern void* kernel_start;
extern void* kernel_end;

void memory_dump_mb_mapping(const struct multiboot_info* multiboot) {
    log_info("Multiboot memory map dump:");
    uintptr_t entry_addr = (uintptr_t) multiboot->mmap_addr;
    uintptr_t entry_end = (uintptr_t) multiboot->mmap_addr + multiboot->mmap_length;
    while (entry_addr < entry_end) {
        const struct multiboot_mmap_entry* entry = (struct multiboot_mmap_entry*) entry_addr;
        uint64_t addr = entry->addr;
        uint64_t size = entry->len;
        log_info("0x%016llX, %llu KiB (%llu pages), type: %u", addr, size / 1024, size / PAGE_SIZE, entry->type);
        entry_addr += entry->size + sizeof(entry->size);
    }

    log_info("Mem lower: %p", (uintptr_t) multiboot->mem_lower * 1000);
    log_info("Mem upper: %p", (uintptr_t) multiboot->mem_upper * 1000);

    log_info("Kernel start address: 0x%08X", &kernel_start);
    log_info("Kernel end address: 0x%08X", &kernel_end);
    uintptr_t kernel_size = ((uintptr_t) &kernel_end - (uintptr_t) &kernel_start);
    log_info("Kernel size: %zu KiB (%zu pages)", kernel_size / 1024, kernel_size / PAGE_SIZE);
}

void memory_init(const struct multiboot_info* multiboot) {
    log_info("Initializing memory");
    memory_dump_mb_mapping(multiboot);
}
