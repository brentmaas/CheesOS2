#ifndef _CHEESOS2_MEMORY_ADDRESS_RANGE_H
#define _CHEESOS2_MEMORY_ADDRESS_RANGE_H

#include <stddef.h>
#include <stdint.h>

struct address_range {
    // The starting address of this address range. Typically a multiple of the page size.
    uintptr_t base;

    // The number of bytes in this address range. Typically a multiple of the page size.
    uintptr_t size;
};

// Compare the base address of two memory ranges.
// Returns:
// - A negative value if `lhs` has a lower base address than `rhs`.
// - Zero if they share the same base addresses.
// - A positive value if `rgs` has a greater base address than `rhs`.
// This function is mainly intended for use with red-black trees.
int address_range_base_cmp(struct address_range* lhs, struct address_range* rhs);

int address_range_size_cmp(struct address_range* lhs, struct address_range* rhs);

// Compare a memory range with an address
// Returns:
// - A negative value if the `lhs` is outside of `rhs` on the low side.
// - Zero if `lhs` is within `rhs`.
// - A postivie value if `lhs` is outside of `rhs` on the high side.
// This function is mainly intended for red-black trees.
int address_range_address_cmp(void* lhs, struct address_range* rhs);

#endif
