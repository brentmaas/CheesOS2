#include "memory/vmm.h"
#include "memory/kernel_layout.h"

#include <stdbool.h>

struct page_directory VMM_ROOT_PAGE_DIR;
struct page_table VMM_KERNEL_PAGE_TABLE;

__attribute__((section(".bootstrap.text")))
struct page_directory* vmm_bootstrap(void) {
    // Get the physical address of the page dir and kernel page table
    struct page_directory* pd = KERNEL_VIRTUAL_TO_PHYSICAL(&VMM_ROOT_PAGE_DIR);
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

    return pd;
}

void vmm_unmap_identity(void) {
    VMM_ROOT_PAGE_DIR.entries[0] = (struct page_dir_entry){
        .present = false
    };
    pt_invalidate_tlb();
}
