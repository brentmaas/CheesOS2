#ifndef _CHEESOS2_MEMORY_ADDRESS_RANGE_H
#define _CHEESOS2_MEMORY_ADDRESS_RANGE_H

#include <stddef.h>
#include <stdint.h>

#include "memory/page_table.h"

// A structure representing a certain range of pages. The main reason this structure represents
// memory areas in pages rather than bytes is to make sure that no overflow occurs when computing the
// index of the page past the last page.
struct page_range {
    // The base page address of this memory range.
    pageaddr_t base;

    // The number of pages in this page range. Note that `pageaddr_t` guarantees that base + size does not overflow
    // for any values for which the page address is within the virtual memory range.
    size_t size;
};

// Compare the base address of two memory ranges.
// Returns:
// - A negative value if `lhs` has a lower base address than `rhs`.
// - Zero if they share the same base addresses.
// - A positive value if `rhs` has a greater base address than `rhs`.
// This function is mainly intended for use with red-black trees.
int page_range_cmp_base(struct page_range* lhs, struct page_range* rhs);

// Compare the size of two memory ranges,
// Returns:
// - A negative value if `lhs` has less pages than `rhs`.
// - Zero if they have the same amount of pages.
// - A positive value if `rhs` has mroe pages than`rhs`.
// This function is mainly intended for use with red-black trees.
int page_range_cmp_size(struct page_range* lhs, struct page_range* rhs);

// Compare a memory range with an address.
// Returns:
// - A negative value if the `lhs` is outside of `rhs` on the low side.
// - Zero if `lhs` is within `rhs`.
// - A postivie value if `lhs` is outside of `rhs` on the high side.
// This function is mainly intended for red-black trees.
int page_range_cmp_address(pageaddr_t lhs, struct page_range* rhs);

#endif
