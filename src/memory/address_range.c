#include "memory/address_range.h"

int address_range_base_cmp(struct address_range* lhs, struct address_range* rhs) {
    if (lhs->base < rhs->base)
        return -1;
    else if (lhs->base > rhs->base)
        return 1;
    return 0;
}

int address_range_size_cmp(struct address_range* lhs, struct address_range* rhs) {
    if (lhs->size < rhs->size)
        return -1;
    else if (lhs->size > rhs->size)
        return 1;
    return 0;
}

int address_range_address_cmp(void* lhs, struct address_range* rhs) {
    uintptr_t addr = (uintptr_t) lhs;
    if (addr < rhs->base)
        return -1;
    if (addr >= rhs->base + rhs->size)
        return 1;
    return 0;
}
