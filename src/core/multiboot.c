#include "core/multiboot.h"
#include "memory/kernel_layout.h"
#include <stddef.h>

#define MULTIBOOT_MAGIC_NUMBER (0x1BADB002)
#define MULTIBOOT_FLAGS (MULTIBOOT_REQUIRE_PAGE_ALIGN | MULTIBOOT_REQUIRE_MEMINFO)

const struct multiboot_header header __attribute__((section(".multiboot"))) = {
    .magic = MULTIBOOT_MAGIC_NUMBER,
    .flags = MULTIBOOT_FLAGS,
    .checksum = -(MULTIBOOT_MAGIC_NUMBER + MULTIBOOT_FLAGS)
};

#define MAX_CMDLINE (256)
#define MAX_BOOT_LOADER_NAME (64)
#define MAX_ENTRIES (64)

static char cmdline[MAX_CMDLINE];
static char boot_loader_name[MAX_BOOT_LOADER_NAME];
static struct multiboot_mmap_entry mmap_entries[MAX_ENTRIES];

static struct multiboot bootstrap_multiboot;

__attribute__((section(".bootstrap.text")))
struct multiboot* multiboot_bootstrap(struct multiboot* multiboot) {
    struct multiboot* physical_bootstrap_multiboot = KERNEL_VIRTUAL_TO_PHYSICAL(&bootstrap_multiboot);
    char* physical_cmdline = KERNEL_VIRTUAL_TO_PHYSICAL(&cmdline);
    char* physical_boot_loader_name = KERNEL_VIRTUAL_TO_PHYSICAL(&boot_loader_name);
    struct multiboot_mmap_entry* physical_mmap_entries = KERNEL_VIRTUAL_TO_PHYSICAL(&mmap_entries);

    // TODO: Add other fields as needed
    *physical_bootstrap_multiboot = (struct multiboot){
        .flags = multiboot->flags & MULTIBOOT_FLAG_MEM_COUNT,
        .mem_lower = multiboot->mem_lower,
        .mem_upper = multiboot->mem_upper,
    };

    if (multiboot->flags & MULTIBOOT_FLAG_CMDLINE) {
        size_t i = 0;
        for (; i < MAX_CMDLINE - 1; ++i) {
            if (!multiboot->cmdline[i]) {
                break;
            }
            physical_cmdline[i] = multiboot->cmdline[i];
        }
        physical_cmdline[i] = 0;
        physical_bootstrap_multiboot->flags |= MULTIBOOT_FLAG_CMDLINE;
        physical_bootstrap_multiboot->cmdline = cmdline;
    }

    if (multiboot->flags & MULTIBOOT_FLAG_BOOT_LOADER_NAME) {
        size_t i = 0;
        for (; i < MAX_BOOT_LOADER_NAME - 1; ++i) {
            if (!multiboot->boot_loader_name[i]) {
                break;
            }
            physical_boot_loader_name[i] = multiboot->boot_loader_name[i];
        }
        physical_boot_loader_name[i] = 0;
        physical_bootstrap_multiboot->flags |= MULTIBOOT_FLAG_BOOT_LOADER_NAME;
        physical_bootstrap_multiboot->boot_loader_name = boot_loader_name;
    }

    if (multiboot->flags & MULTIBOOT_FLAG_MEMORY_MAP) {
        uintptr_t entry_addr = (uintptr_t) multiboot->mmap_addr;
        uintptr_t entry_end = (uintptr_t) multiboot->mmap_addr + multiboot->mmap_length;
        size_t i = 0;

        while (i < MAX_ENTRIES && entry_addr < entry_end) {
            const struct multiboot_mmap_entry* entry = (struct multiboot_mmap_entry*) entry_addr;
            physical_mmap_entries[i++] = (struct multiboot_mmap_entry){
                .size = sizeof(struct multiboot_mmap_entry) - sizeof(entry->size),
                .addr = entry->addr,
                .len = entry->len,
                .type = entry->type
            };
            entry_addr += entry->size + sizeof(entry->size);
        }

        physical_bootstrap_multiboot->flags |= MULTIBOOT_FLAG_MEMORY_MAP;
        physical_bootstrap_multiboot->mmap_length = i * sizeof(struct multiboot_mmap_entry);
        physical_bootstrap_multiboot->mmap_addr = mmap_entries;
    }

    return &bootstrap_multiboot;
}
