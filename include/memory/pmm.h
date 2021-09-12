#ifndef _CHEESOS2_MEMORY_PMM_H
#define _CHEESOS2_MEMORY_PMM_H

#include "core/multiboot.h"

#include <stddef.h>

// The physical memory manager keeps a cache of physical pages so that a new page may
// be assigned relatively quickly in most cases. This is the number of elements in it.
#define PMM_PAGE_STACK_ENTRIES (1024U)

// Initialize the physical memory manager, according to the memory map provided in `multiboot`.
// This function assumes that there is no virtual memory mapped other than the kernel.
// Initially, the internal cache of pages will be cleared. Additional memory areas can be mapped
// as reserved, but this will cause the cache to be reset, and so it is most efficient to reserve
// those areas before calling any allocation routines.
void pmm_init(const struct multiboot* mb);

// Return the number of pages currently available for allocation.
size_t pmm_free_pages(void);

// Return the total number of pages the physical allocator keeps track of.
// Note: This may include pages which can never be free (bad memory or not present at all).
size_t pmm_total_pages(void);

// Allocate a physical page.
// Note that this only concerns *physical* pages, and the page is not mapped into virtual memory.
// Returns the *physical* page address.
// TODO: Describe error conditions.
void* pmm_alloc_page(void);

// Return the physical page to the system memory pool.
// Note that this only concernts *physical* pages. `page` must be the *physical* address of the
// page to free, aligned to `PAGE_SIZE`.
void pmm_free_page(void* page);

#endif
