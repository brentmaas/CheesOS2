#include "memory/memory.h"
#include "memory/kernel_layout.h"
#include "memory/alloc/buddy.h"

#include "core/panic.h"
#include "debug/log.h"
#include "debug/assert.h"

#include <stdint.h>
#include <stdbool.h>

static struct page_directory kernel_page_dir;
static struct page_table kernel_page_table;

static struct buddy_allocator main_physical_allocator;

__attribute__((section(".bootstrap.text")))
struct page_directory* memory_bootstrap() {
    // Get the physical address of the page dir and kernel page table
    struct page_directory* pd = KERNEL_VIRTUAL_TO_PHYSICAL(&kernel_page_dir);
    struct page_table* pt = KERNEL_VIRTUAL_TO_PHYSICAL(&kernel_page_table);

    for (size_t i = 0; i < PAGE_DIR_ENTRY_COUNT; ++i) {
        pd->entries[i] = (struct page_dir_entry){};
    }

    for (size_t i = 0; i < PAGE_TABLE_ENTRY_COUNT; ++i) {
        pt->entries[i] = (struct page_table_entry){};
    }

    // Identity map to prevent generating page faults when enabling paging.
    // To be removed after
    pd->entries[0] = (struct page_dir_entry){
        .present = true,
        .write_enable = true,
        .user = true,
        .page_table_address = (uintptr_t) pt / PAGE_SIZE
    };

    // The actual kernel page table.
    pd->entries[PAGE_DIR_INDEX(kernel_virtual_start_addr())] = (struct page_dir_entry){
        .present = true,
        .write_enable = true,
        .user = true,
        .page_table_address = (uintptr_t) pt / PAGE_SIZE
    };

    uintptr_t kernel_end_page = PAGE_ALIGN_FORWARD((uintptr_t) &kernel_end) / PAGE_SIZE;

    // Add everything up to kernel_end_addr to the kernel page table
    // kernel.ld guarantees that kernel_end_page fits in the page table
    for (size_t i = 0; i < kernel_end_page; ++i) {
        pt->entries[i] = (struct page_table_entry){
            .present = true,
            .write_enable = true,
            .user = true,
            .page_address = i
        };
    }

    return pd;
}

static void memory_dump_map(const struct multiboot_info* multiboot) {
    log_debug("Multiboot memory map dump:");
    uintptr_t entry_addr = (uintptr_t) multiboot->mmap_addr;
    uintptr_t entry_end = (uintptr_t) multiboot->mmap_addr + multiboot->mmap_length;
    while (entry_addr < entry_end) {
        const struct multiboot_mmap_entry* entry = (struct multiboot_mmap_entry*) entry_addr;
        uint64_t addr = entry->addr;
        uint64_t size = entry->len;
        log_debug("0x%llX, %llu KiB (%llu pages), type: %u", addr, size / 1024, size / PAGE_SIZE, entry->type);
        entry_addr += entry->size + sizeof(entry->size);
    }
}

static const struct multiboot_mmap_entry* memory_find_mb_mmap(
    const struct multiboot_info* multiboot,
    uintptr_t start_addr,
    uintptr_t end_addr
) {
    uintptr_t entry_addr = (uintptr_t) multiboot->mmap_addr;
    uintptr_t entry_end = (uintptr_t) multiboot->mmap_addr + multiboot->mmap_length;
    while (entry_addr < entry_end) {
        const struct multiboot_mmap_entry* entry = (struct multiboot_mmap_entry*) entry_addr;
        uint64_t map_start = entry->addr;
        uint64_t map_end = map_start + entry->len;

        if (map_start <= (uint64_t) start_addr && map_end >= (uint64_t) end_addr) {
            return entry;
        }

        entry_addr += entry->size + sizeof(entry->size);
    }

    return NULL;
}

enum memory_result memory_init(const struct multiboot_info* multiboot) {
    log_info("Initializing memory");

    memory_dump_map(multiboot);

    // Remove the identity map
    kernel_page_dir.entries[0] = (struct page_dir_entry){
        .present = false
    };

    paging_invalidate_tlb();

    uintptr_t kernel_start_addr = (uintptr_t) &kernel_start;
    assert(IS_PAGE_ALIGNED(kernel_start_addr));
    uintptr_t kernel_end_addr = PAGE_ALIGN_FORWARD((uintptr_t) &kernel_end);
    size_t kernel_size = kernel_end_addr - kernel_start_addr;

    log_info(
        "Kernel memory zone: %p to %p, %zu KiB (%zu pages)",
        (void*) kernel_start_addr,
        (void*) kernel_end_addr,
        kernel_size / 1024,
        kernel_size / PAGE_SIZE
    );

    // Attempt to find the 'main' memory zone: The multiboot mapping which completely
    // contains the kernel. The remainder of that memory mapping (everything > kernel_start_addr)
    // will be used for physical memory allocation.
    // TODO: Handle memory holes
    // TODO: Handle < kernel_start_addr zone
    const struct multiboot_mmap_entry* kernel_entry = memory_find_mb_mmap(multiboot, kernel_start_addr, kernel_end_addr);
    if (!kernel_entry) {
        log_error("Multiboot reports no memory map which completely contains the kernel");
        memory_dump_map(multiboot);
        return MEMORY_NO_VALID_ZONE;
    } else if (kernel_entry->type != MULTIBOOT_MMAP_AVAILABLE) {
        log_error("Multiboot memory map containing kernel is not reported as available memory");
        memory_dump_map(multiboot);
        return MEMORY_NO_VALID_ZONE;
    }

    uint64_t kernel_entry_end = kernel_entry->addr + kernel_entry->len;
    if (kernel_entry_end >= 0x100000000ULL) {
        kernel_entry_end = 0x100000000ULL; // Limit to 4 GiB
    }
    kernel_entry_end = PAGE_ALIGN_BACKWARD(kernel_entry_end);
    size_t remaining_mem = (size_t) (kernel_entry_end - (uint64_t) kernel_end_addr);
    log_info(
        "Physical allocation zone: %p to %p, %zu KiB (%zu pages)",
        (void*) kernel_end_addr,
        (void*) (uintptr_t) (kernel_entry_end),
        remaining_mem / 1024,
        remaining_mem / PAGE_SIZE
    );

    // TODO: Rework
    // buddy_init(&main_physical_allocator, (void*) kernel_end_addr, remaining_mem);

    return MEMORY_OK;
}

void* memory_alloc_physical(size_t pages) {
    return buddy_alloc_pages(&main_physical_allocator, pages);
}

void memory_free_physical(void* memory) {
    buddy_free_pages(&main_physical_allocator, memory);
}

enum memory_result memory_set_mapping(struct page_directory* pdir, void* virtual, void* physical, enum memory_map_flags flags) {
    uintptr_t virtual_addr = (uintptr_t) virtual;
    uintptr_t physical_addr = (uintptr_t) physical;
    assert(IS_PAGE_ALIGNED(virtual_addr));
    assert(IS_PAGE_ALIGNED(physical_addr));

    size_t pdi = PAGE_DIR_INDEX(virtual_addr);
    size_t pti = PAGE_TABLE_INDEX(virtual_addr);

    struct page_dir_entry* pde = &pdir->entries[pdi];
    struct page_table* pt;
    if (pde->present) {
        pt = (struct page_table*) (pde->page_table_address * PAGE_SIZE);
    } else {
        pt = memory_alloc_physical(1);
        if (!pt) {
            return MEMORY_PHYSICAL_ALLOC_FAILED;
        }

        *pde = (struct page_dir_entry) {
            .present = true,
            .user = true,
            .page_table_address = ((uintptr_t) pt) / PAGE_SIZE
        };
    }

    struct page_table_entry* pte = &pt->entries[pti];
    *pte = (struct page_table_entry) {
        .present = true,
        .write_enable = (flags & MEMORY_MAP_WRITABLE) != 0,
        .user = (flags & MEMORY_MAP_USER) != 0,
        .page_address = ((uintptr_t) physical) / PAGE_SIZE
    };

    paging_invalidate_address(virtual);
    return MEMORY_OK;
}

enum memory_result memory_map(struct page_directory* pdir, void* virtual, enum memory_map_flags flags) {
    void* physical = memory_alloc_physical(1);
    if (!physical) {
        return MEMORY_PHYSICAL_ALLOC_FAILED;
    }

    int result = memory_set_mapping(pdir, virtual, physical, flags);
    if (result) {
        memory_free_physical(physical);
    }

    return result;
}

enum memory_result memory_unmap(struct page_directory* pdir, void* virtual) {
    uintptr_t virtual_addr = (uintptr_t) virtual;
    assert(IS_PAGE_ALIGNED(virtual_addr));

    size_t pdi = PAGE_DIR_INDEX(virtual_addr);
    size_t pti = PAGE_TABLE_INDEX(virtual_addr);

    struct page_dir_entry* pde = &pdir->entries[pdi];
    if (!pde->present) {
        return MEMORY_UNMAPPED_VADDR;
    }
    struct page_table* pt = (struct page_table*) (pde->page_table_address * PAGE_SIZE);
    struct page_table_entry* pte = &pt->entries[pti];
    if (!pte->present) {
        return MEMORY_UNMAPPED_VADDR;
    }

    *pte = (struct page_table_entry) {.present = false};
    paging_invalidate_address(virtual);

    for (size_t i = 0; i < PAGE_TABLE_ENTRY_COUNT; ++i) {
        if (pt->entries[i].present) {
            return MEMORY_OK;
        }
    }

    // Remove page table from the page directory, as its empty
    *pde = (struct page_dir_entry) {.present = false};

    memory_free_physical(pt);
    return MEMORY_OK;
}

enum memory_result memory_translate(const struct page_directory* pdir, void* virtual, void** physical) {
    uintptr_t virtual_addr = (uintptr_t) virtual;

    size_t pdi = PAGE_DIR_INDEX(virtual_addr);
    size_t pti = PAGE_TABLE_INDEX(virtual_addr);

    const struct page_dir_entry* pde = &pdir->entries[pdi];
    if (!pde->present) {
        return MEMORY_UNMAPPED_VADDR;
    }
    const struct page_table* pt = (struct page_table*) (pde->page_table_address * PAGE_SIZE);
    const struct page_table_entry* pte = &pt->entries[pti];
    if (!pte->present) {
        return MEMORY_UNMAPPED_VADDR;
    }

    *physical = (void*) (pte->page_address * PAGE_SIZE + PAGE_OFFSET(virtual_addr));
    return MEMORY_OK;
}
