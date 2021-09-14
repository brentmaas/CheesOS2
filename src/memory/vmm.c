#include "memory/vmm.h"
#include "memory/kernel_layout.h"

#include "debug/log.h"

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

void vmm_unmap_page(void* virtual) {
    uintptr_t addr = (uintptr_t) virtual;
    size_t pdi = PAGE_DIR_INDEX(addr);
    size_t pti = PAGE_TABLE_INDEX(addr);

    struct vmm_recursive_page_table* rpt = vmm_current_page_table();

    // First, check if the page table is mapped at all.
    struct page_directory* pd = &rpt->page_directory;
    struct page_dir_entry* pde = &pd->entries[pdi];
    if (!pde->present) {
        log_debug("Tried to unmap %p which was not mapped", virtual);
        // TODO: Return some sort of error?
        return;
    }

    // Check if the page is mapped at all
    struct page_table* pt = &rpt->page_tables[pdi];
    struct page_table_entry* pte = &pt->entries[pti];
    if (!pte->present) {
        log_debug("Tried to unmap %p which was not mapped", virtual);
        // TODO: Return some sort of error?
        return;
    }

    // remove the page from the table
    *pte = (struct page_table_entry){};
    pt_invalidate_address(virtual);

    // TODO: Maybe free page table if it's empty
}
