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
// Returns a physical page index on success, or a negative value on failure.
intptr_t pmm_alloc(void);

// Return a physical page index to the system memory pool.
void pmm_free(uintptr_t page);

#endif
