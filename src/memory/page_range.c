#include "memory/page_range.h"

int page_range_cmp_base(struct page_range* lhs, struct page_range* rhs) {
    if (lhs->base < rhs->base)
        return -1;
    else if (lhs->base > rhs->base)
        return 1;
    return 0;
}

int page_range_cmp_size(struct page_range* lhs, struct page_range* rhs) {
    if (lhs->size < rhs->size)
        return -1;
    else if (lhs->size > rhs->size)
        return 1;
    return 0;
}

int page_range_cmp_address(pageaddr_t lhs, struct page_range* rhs) {
    pageaddr_t addr = (pageaddr_t) lhs;
    if (addr < rhs->base)
        return -1;
    if (addr >= rhs->base + rhs->size)
        return 1;
    return 0;
}
