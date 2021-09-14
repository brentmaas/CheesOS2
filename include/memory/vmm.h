#ifndef _CHEESOS2_MEMORY_VMM_H
#define _CHEESOS2_MEMORY_VMM_H

#include "memory/page_table.h"

// The page directory index in which the page directory is mapped to itself.
// This should be some place other than where the kernel itself is going to be placed,
// which is placed at address 0xC000000, index 768.
#define VMM_RECUSIVE_PAGE_DIR_INDEX (PAGE_TABLE_ENTRY_COUNT - 1)

// A structure representing the layout of the recusive page directory
struct __attribute__((packed, aligned(PAGE_SIZE * 1024))) vmm_recursive_page_table {
    struct page_table page_tables[PAGE_TABLE_ENTRY_COUNT - 1];
    struct page_directory page_directory;
};

// Bootstrapping memory identity maps kernel memory. This function removes that mapping.
void vmm_unmap_identity();

// To ease management of the page table, a recursive page table is used. This function retrieves the location of the
// recursive page table.
struct vmm_recursive_page_table* vmm_current_page_table();

// Unmap a virtual address
void vmm_unmap_page(void* virtual);

#endif
