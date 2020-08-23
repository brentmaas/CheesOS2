#include "memory/alloc/buddy.h"
#include "memory/align.h"
#include "memory/paging.h"

#include "debug/assert.h"
#include "debug/log.h"

#include <stdbool.h>
#include <string.h>

#define CEIL_DIV(a, b) ((a) / (b) + ((a) % (b) != 0))

static size_t bits_requires_for_order(size_t num_pages, size_t order) {
    // Floor division, as there is no allocation information required
    // for partial pages.

    return num_pages / (1 << order);
}

static size_t bytes_requires_for_order(size_t num_pages, size_t order) {
    // Make sure the number of bits rounds up to a multiple of 8
    // so that each order can have a pointer instead of an offset.
    return CEIL_DIV(bits_requires_for_order(num_pages, order), 8);
}

static size_t bit_pages_required_for_pages(size_t num_pages) {
    size_t total_bytes = 0;
    for (size_t i = 0; i < BUDDY_ORDERS; ++i) {
        total_bytes += bytes_requires_for_order(num_pages, i);
    }

    return CEIL_DIV(total_bytes, PAGE_SIZE);
}

static void bitmap_set(uint8_t* bitmap, size_t index, bool value) {
    if (value) {
        bitmap[index / 8] |= 1 << (index % 8);
    } else {
        bitmap[index / 8] &= ~(1 << (index % 8));
    }
}

static bool bitmap_get(const uint8_t* bitmap, size_t index) {
    return (bitmap[index / 8] >> (index % 8)) & 1;
}

static void buddy_push_free_page(struct buddy_allocator* buddy, size_t order, struct buddy_free_page* new_head) {
    assert(IS_ALIGNED_TO((uintptr_t) new_head - buddy->first_allocatable_page, (1 << order) * PAGE_SIZE));

    struct buddy_free_page* old_head = buddy->orders[order].head;
    assert(!old_head || !old_head->prev);

    new_head->prev = NULL;
    new_head->next = old_head;
    if (old_head) {
        old_head->prev = new_head;
    }

    buddy->orders[order].head = new_head;
}

static struct buddy_free_page* buddy_pop_free_page(struct buddy_allocator* buddy, size_t order) {
    struct buddy_free_page* old_head = buddy->orders[order].head;
    if (!old_head) {
        return NULL;
    }

    struct buddy_free_page* new_head = old_head->next;
    if (new_head) {
        new_head->prev = NULL;
    }

    buddy->orders[order].head = new_head;
    return old_head;
}

static void buddy_remove_free_page(struct buddy_allocator* buddy, size_t order, struct buddy_free_page* node) {
    if (node == buddy->orders[order].head) {
        struct buddy_free_page* new_head = node->next;
        buddy->orders[order].head = new_head;
        if (new_head) {
            new_head->prev = NULL;
        }
        return;
    }

    struct buddy_free_page* prev = node->prev;
    struct buddy_free_page* next = node->next;

    if (prev) {
        prev->next = next;
    }

    if (next) {
        next->prev = prev;
    }
}

void buddy_init(struct buddy_allocator* buddy, void* base, size_t len) {
    uintptr_t base_addr_aligned = PAGE_ALIGN_FORWARD((uintptr_t) base);
    uintptr_t end_addr_aligned = PAGE_ALIGN_BACKWARD((uintptr_t) base + len);
    size_t len_aligned = end_addr_aligned - base_addr_aligned;

    buddy->base_addr = base_addr_aligned;
    buddy->total_pages = len_aligned / PAGE_SIZE;

    // The initial guess of the number of pages required for the bitmap ssumes the entire
    // memory range are allocatable pages, but that doesn't factor in the number of pages
    // required for the bitmap itself.
    // To fix this, simply calculate the required pages for the new number of bits. This can of course
    // yield an incorrect size again, so repeat the process so long as the page guess decreases.
    // This seems to converge pretty fast.
    size_t bitmap_pages = bit_pages_required_for_pages(buddy->total_pages);

    while (true) {
        size_t new_guess = bit_pages_required_for_pages(buddy->total_pages - bitmap_pages);
        if (new_guess >= bitmap_pages) {
            break;
        }
        bitmap_pages = new_guess;
    }

    buddy->first_allocatable_page = base_addr_aligned + bitmap_pages * PAGE_SIZE;
    buddy->num_allocatable_pages = buddy->total_pages - bitmap_pages;

    // Mark all pages as deallocated
    uintptr_t bitmap = base_addr_aligned;
    memset((uint8_t*) bitmap, 0, bitmap_pages * PAGE_SIZE);

    // Initialize orders.
    for (size_t i = 0; i < BUDDY_ORDERS; ++i) {
        buddy->orders[i].head = NULL;
        buddy->orders[i].bitmap = (uint8_t*) bitmap;
        bitmap += bytes_requires_for_order(buddy->num_allocatable_pages, i);
    }

    // Add pages to the free lists.
    // Starting with the largest buddy, keep greedily adding blocks to the free lists
    // until none remain.
    size_t page_index = 0;
    size_t order = BUDDY_ORDERS;
    while (order > 0) {
        --order;
        size_t order_pages = 1 << order;
        size_t n_registered = 0;
        while (page_index + order_pages <= buddy->num_allocatable_pages) {
            uintptr_t block_addr = buddy->first_allocatable_page + page_index * PAGE_SIZE;
            buddy_push_free_page(buddy, order, (struct buddy_free_page*) block_addr);
            page_index += order_pages;
            ++n_registered;
        }
    }
}

static uintptr_t buddy_block_index(struct buddy_allocator* buddy, size_t order, void* block) {
    uintptr_t block_bytes = (1 << order) * PAGE_SIZE;
    uintptr_t block_index = ((uintptr_t) block - buddy->first_allocatable_page) / block_bytes;
    assert(block_bytes * block_index + buddy->first_allocatable_page == (uintptr_t) block);
    return block_index;
}

static bool buddy_is_allocated(struct buddy_allocator* buddy, size_t order, void* block) {
    uintptr_t block_index = buddy_block_index(buddy, order, block);
    return bitmap_get(buddy->orders[order].bitmap, block_index);
}

static void buddy_mark_allocated(struct buddy_allocator* buddy, size_t order, void* block) {
    uintptr_t block_index = buddy_block_index(buddy, order, block);
    assert(bitmap_get(buddy->orders[order].bitmap, block_index) == false);
    bitmap_set(buddy->orders[order].bitmap, block_index, true);
}

static void buddy_mark_free(struct buddy_allocator* buddy, size_t order, void* block) {
    uintptr_t block_index = buddy_block_index(buddy, order, block);
    assert(bitmap_get(buddy->orders[order].bitmap, block_index) == true);
    bitmap_set(buddy->orders[order].bitmap, block_index, false);
}

static void* buddy_alloc_from_order(struct buddy_allocator* buddy, size_t order) {
    void* block = buddy_pop_free_page(buddy, order);
    if (!block) {
        return NULL;
    }

    buddy_mark_allocated(buddy, order, block);
    return block;
}

// Returns and marks the first block as allocated, and adds the second block to the
// free list of one order smaller. The returned block is also an order smaller.
void* buddy_split(struct buddy_allocator* buddy, void* block, size_t order) {
    assert(order >= 1);
    size_t child_order = order - 1;

    size_t child_block_pages = 1 << child_order;
    size_t child_block_bytes = child_block_pages * PAGE_SIZE;

    void* first = block;
    void* second = (void*) ((uintptr_t) block + child_block_bytes);

    buddy_mark_allocated(buddy, child_order, first);
    assert(!buddy_is_allocated(buddy, child_order, second));

    buddy_push_free_page(buddy, child_order, second);

    return first;
}

void* buddy_alloc_pages(struct buddy_allocator* buddy, size_t order) {
    assert(order < BUDDY_ORDERS);

    void* block;
    size_t block_order = order;
    for (; block_order < BUDDY_ORDERS; ++block_order) {
        block = buddy_alloc_from_order(buddy, block_order);
        if (block) {
            break;
        }
    }

    if (!block) {
        // There was no space for the allocation at all
        return NULL;
    }

    while (block_order > order) {
        block = buddy_split(buddy, block, block_order);
        --block_order;
    }

    memset(block, 0, (1 << order) * PAGE_SIZE);
    return block;
}

void buddy_free_pages(struct buddy_allocator* buddy, void* block) {
    if (!block) {
        return;
    }

    assert(IS_PAGE_ALIGNED((uintptr_t) block));
    assert((uintptr_t) block >= buddy->first_allocatable_page);
    assert((uintptr_t) block < buddy->first_allocatable_page + buddy->num_allocatable_pages * PAGE_SIZE);
    // Obtain the order simply by checking what is the smallest allocated block.
    size_t order = 0;
    for (; order < BUDDY_ORDERS; ++order) {
        if (buddy_is_allocated(buddy, order, block)) {
            break;
        }
    }

    // If this assertion fails, there was either a double free or an invalid memory area was freed.
    assert(order < BUDDY_ORDERS);

    while (true) {
        buddy_mark_free(buddy, order, block);

        if (order == BUDDY_ORDERS - 1) {
            // A block of the largest order has no buddy
            break;
        }

        size_t block_bytes = (1 << order) * PAGE_SIZE;

        uintptr_t block_index = buddy_block_index(buddy, order, block);
        uintptr_t buddy_block_index = block_index ^ 1;

        intptr_t diff = block_bytes * (buddy_block_index - block_index);
        uintptr_t buddy_block_addr = (uintptr_t) block + block_bytes * (buddy_block_index - block_index);

        if (buddy_block_addr + block_bytes >= buddy->first_allocatable_page + buddy->num_allocatable_pages * PAGE_SIZE) {
            // The buddy block is out of range.
            break;
        }

        bool buddy_allocated = bitmap_get(buddy->orders[order].bitmap, buddy_block_index);
        if (buddy_allocated) {
            // Cannot merge anymore.
            break;
        }

        buddy_remove_free_page(buddy, order, (void*) buddy_block_addr);

        // Go up to the parent block.
        ++order;
        if (diff < 0) {
            block = (void*) buddy_block_addr;
        }
    }

    buddy_push_free_page(buddy, order, block);
}

void buddy_dump_stats(const struct buddy_allocator* buddy) {
    for (size_t i = 0; i < BUDDY_ORDERS; ++i) {
        struct buddy_free_page* head = buddy->orders[i].head;
        size_t count = 0;

        while (head) {
            ++count;
            head = head->next;
        }

        log_debug("Order %zu: %zu page(s) in free list\n", i, count);
    }
}
