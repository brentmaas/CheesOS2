#ifndef _CHEESOS2_MEMORY_ALLOC_BUDDY_H
#define _CHEESOS2_MEMORY_ALLOC_BUDDY_H

#include <stdint.h>
#include <stddef.h>

#define BUDDY_ORDERS (8) // Maximum allocations of 2^7 pages (= 512 KiB) at once.

struct buddy_free_page {
    struct buddy_free_page* next;
    struct buddy_free_page* prev;
};

struct buddy_order {
    uint8_t* bitmap;

    // The first of the sequence of free areas of this order.
    struct buddy_free_page* head;
};

struct buddy_allocator {
    // The bitmap lies in the first part of the allocator's memoy
    // region, and pages are allocated from everything behind it.
    size_t base_addr; // Aligned to PAGE_SIZE.

    // The total number of pages (including bitmap) this allocator manages.
    size_t total_pages;

    // The first page which may be returned by allocation functions.
    // Everything from `bitmap` to `first_allocatable_page` is reserved for the
    // bitmap.
    uintptr_t first_allocatable_page; // Multiple of PAGE_SIZE

    // The number of pages of which regions may be returned by the allocation functions.
    size_t num_allocatable_pages;

    struct buddy_order orders[BUDDY_ORDERS];
};

void buddy_init(struct buddy_allocator* buddy, void* base, size_t len);
void* buddy_alloc_pages(struct buddy_allocator* buddy, size_t pages);
void* buddy_alloc_order(struct buddy_allocator* buddy, size_t order);
void buddy_free_pages(struct buddy_allocator* buddy, void* block);
void buddy_dump_stats(const struct buddy_allocator* buddy);

#endif
