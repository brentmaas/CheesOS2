#ifndef _CHEESOS2_MEMORY_VIRTUAL_ADDRESS_ALLOCATOR_H
#define _CHEESOS2_MEMORY_VIRTUAL_ADDRESS_ALLOCATOR_H

#include "memory/page_table.h"
#include "memory/page_range.h"
#include "utility/containers/rbtree.h"

#include <stdbool.h>
#include <stdint.h>

struct address_allocator_hole {
    struct rb_node by_base_node;
    struct rb_node by_size_node;
    struct page_range range;
};

struct address_allocator {
    // Free holes, by base address. This makes neighboring holes deallocation fast.
    struct rb_tree holes_by_base;

    // Free holes, by size. This makes picking the right sized hole during allocation fast.
    struct rb_tree holes_by_size;

    // A single scratch node, mainly used so that calls to `address_allocator_init` do not need to
    // perform any allocation themselves.
    struct address_allocator_hole scratch;
};

void address_allocator_init(struct address_allocator* aa, struct page_range range);

// Allocate an address somewhere in the available ranges. If this could not be done,
// returns false.
// Important: To make this function feasable for use during virtual memory allocation (in the vmm),
// **this function should never allocate**.
bool address_allocator_alloc_anywhere(struct address_allocator* aa, pageaddr_t* base_out, size_t size);

// Attempt to allocate an address at a fixed location. If this could not be done, returns false.
// Note: in contrast to `address_allocator_alloc_anywhere`, this function may perform allocating calls to vmm_*.
bool address_allocator_alloc_fixed(struct address_allocator* aa, struct page_range range);

// Deallocate a previously allocated page range.
// Note: The page range does not need to have come from any single call to address_allocator_alloc_*,
// as long as all the pages in the range are continuously allocated.
// Note: in contrast to `address_allocator_alloc_anywhere`, this function may perform allocating calls to vmm_*.
// Asserts that all pages are allocated.
void address_allocator_free(struct address_allocator* aa, struct page_range range);

void address_allocator_dump(struct address_allocator* aa);

#endif
