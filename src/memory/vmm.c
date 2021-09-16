#include "memory/vmm.h"
#include "memory/pmm.h"
#include "memory/kernel_layout.h"

#include "debug/log.h"
#include "debug/assert.h"

#include <stdbool.h>

static struct page_directory VMM_KERNEL_PAGE_DIR;
static struct page_table VMM_KERNEL_PAGE_TABLE;

__attribute__((section(".bootstrap.text")))
struct page_directory* vmm_bootstrap(void) {
    // Get the physical address of the page dir and kernel page table
    struct page_directory* pd = KERNEL_VIRTUAL_TO_PHYSICAL(&VMM_KERNEL_PAGE_DIR);
    struct page_table* pt = KERNEL_VIRTUAL_TO_PHYSICAL(&VMM_KERNEL_PAGE_TABLE);

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
        .page_table_address = PAGE_INDEX((uintptr_t) pt)
    };

    // The actual kernel page table.
    pd->entries[PAGE_DIR_INDEX(KERNEL_VIRTUAL_START)] = (struct page_dir_entry){
        .present = true,
        .write_enable = true,
        .page_table_address = PAGE_INDEX((uintptr_t) pt)
    };

    uintptr_t kernel_end_page = PAGE_INDEX(PAGE_ALIGN_FORWARD(KERNEL_PHYSICAL_END));

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

    // Recursively map the page directory to itself
    pd->entries[VMM_RECUSIVE_PAGE_DIR_INDEX] = (struct page_dir_entry){
        .present = true,
        .write_enable = true,
        .page_table_address = PAGE_INDEX((uintptr_t) pd),
    };

    return pd;
}

void vmm_unmap_identity(void) {
    VMM_KERNEL_PAGE_DIR.entries[0] = (struct page_dir_entry){};
    pt_invalidate_tlb();
}

struct vmm_recursive_page_table* vmm_current_page_table() {
    return (struct vmm_recursive_page_table*) (VMM_RECUSIVE_PAGE_DIR_INDEX * PAGE_TABLE_ENTRY_COUNT * PAGE_SIZE);
}

enum vmm_result vmm_map_page(void* virtual, void* physical, enum vmm_map_flags flags) {
    uintptr_t vaddr = (uintptr_t) virtual;
    assert(IS_PAGE_ALIGNED(vaddr));

    uintptr_t paddr = (uintptr_t) physical;
    assert(IS_PAGE_ALIGNED(paddr));

    size_t pdi = PAGE_DIR_INDEX(vaddr);
    size_t pti = PAGE_TABLE_INDEX(vaddr);

    struct vmm_recursive_page_table* rpt = vmm_current_page_table();

    struct page_dir_entry* pde = &rpt->page_directory.entries[pdi];
    if (!pde->present) {
        // No page table available, so allocate one.

        intptr_t page_table_page = pmm_alloc();
        if (page_table_page < 0)
            return VMM_OUT_OF_PHYSICAL_MEMORY;

        *pde = (struct page_dir_entry){
            .present = true,
            .page_table_address = page_table_page,
        };
    }

    struct page_table_entry* pte = &rpt->page_tables[pdi].entries[pti];
    if (pte->present && (flags & VMM_MAP_OVERWRITE) == 0) {
        // Page was previously mapped.
        return VMM_ALREADY_MAPPED;
    }

    *pte = (struct page_table_entry){
        .present = true,
        .write_enable = (flags & VMM_MAP_WRITABLE) != 0,
        .user = (flags & VMM_MAP_USER) != 0,
        .page_address = PAGE_INDEX(paddr)
    };

    if (flags & VMM_MAP_OVERWRITE) {
        pt_invalidate_address(virtual);
    }

    return VMM_SUCCESS;
}

enum vmm_result vmm_unmap_page(void* virtual) {
    uintptr_t vaddr = (uintptr_t) virtual;
    size_t pdi = PAGE_DIR_INDEX(vaddr);
    size_t pti = PAGE_TABLE_INDEX(vaddr);

    struct vmm_recursive_page_table* rpt = vmm_current_page_table();

    // First, check if the page table is mapped at all.
    struct page_dir_entry* pde = &rpt->page_directory.entries[pdi];
    if (!pde->present) {
        log_warn("Tried to unmap %p which was not mapped", virtual);
        return VMM_NOT_MAPPED;
    }

    // Check if the page is mapped at all
    struct page_table_entry* pte = &rpt->page_tables[pdi].entries[pti];
    if (!pte->present) {
        log_warn("Tried to unmap %p which was not mapped", virtual);
        return VMM_NOT_MAPPED;
    }

    // remove the page from the table
    *pte = (struct page_table_entry){};
    pt_invalidate_address(virtual);

    // TODO: Maybe free page table if it's empty
    return VMM_SUCCESS;
}

enum vmm_result vmm_translate(void* virtual, void** physical) {
    uintptr_t vaddr = (uintptr_t) virtual;
    size_t pdi = PAGE_DIR_INDEX(vaddr);
    size_t pti = PAGE_TABLE_INDEX(vaddr);

    struct vmm_recursive_page_table* rpt = vmm_current_page_table();

    struct page_dir_entry* pde = &rpt->page_directory.entries[pdi];
    if (!pde->present) {
        return VMM_NOT_MAPPED;
    }

    struct page_table_entry* pte = &rpt->page_tables[pdi].entries[pti];
    if (!pte->present) {
        return VMM_NOT_MAPPED;
    }

    *physical = (void*)((pte->page_address << PAGE_OFFSET_BITS) | PAGE_OFFSET(vaddr));

    return VMM_SUCCESS;
}
