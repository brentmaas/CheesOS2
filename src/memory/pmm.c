#include "memory/pmm.h"
#include "memory/align.h"
#include "memory/memory.h"
#include "memory/page_table.h"
#include "memory/kernel_layout.h"

#include "debug/assert.h"
#include "debug/log.h"
#include "core/panic.h"

#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include <string.h>

// Required in order to avoid some expensive 64-bit division routines.
_Static_assert(CHAR_BIT == 8);
#define CHAR_BIT_LOG2 (3)

// find_bitmap_area returns this value when there was no suitable location.
// Note: this value must not be page aligned so that it cannot represent
// a valid address.
#define FIND_BITMAP_FAILED ((uint8_t*) 0xFFFFFFFFULL)

#define BITMAP_BITS_PER_PAGE (PAGE_SIZE * CHAR_BIT)

// The maximum pages of system memory.
// Note: Specific for 32-bit systems
#define MAX_PAGES (PAGE_DIR_ENTRY_COUNT * PAGE_TABLE_ENTRY_COUNT)

// The absolute maximum number of pages that the bitmap can be on a 32-bit system.
#define MAX_BITMAP_PAGES (MAX_PAGES / BITMAP_BITS_PER_PAGE)

static struct {
    // The total amount of pages the system has to keep track of.
    // Physical pages are assumed to exist for addresses 0 < x < `pages`.
    // Note that memory which is not allowed to be used is simply marked as reserved
    // here.
    size_t total_pages;

    // The total amount of pages currently available for allocation.
    size_t free_pages;

    // Number of elements currently in the page stack.
    size_t page_stack_top;

    // Stack used to quickly find new pages.
    uintptr_t page_stack[PMM_PAGE_STACK_ENTRIES];
} PMM_STATE;

// The actual bitmap for allocation state. Note that on many systems with less memory,
// this hosts way too many pages. Still, its easy for us to allocate memory this way, as
// multiboot makes sure that it is available for us. This does require the system to have
// at least a few hundred KB of memory to boot. Memory which isn't used is marked as free
// after the physical memory allocator is started.
// This buffer is page aligned to make freeing the unused memory more efficient.
uint8_t BITMAP[MAX_BITMAP_PAGES * PAGE_SIZE] __attribute__((aligned(PAGE_SIZE)));

// Compute the total number of pages that the system has to keep track of.
// This entails the number of pages from physical address 0 to the physical page
// with the largest address.
static size_t compute_total_pages(const struct multiboot* mb) {
    uint64_t max_addr = 0;

    uintptr_t entry_addr = (uintptr_t) mb->mmap_addr;
    uintptr_t entry_end = (uintptr_t) mb->mmap_addr + mb->mmap_length;
    while (entry_addr < entry_end) {
        const struct multiboot_mmap_entry* entry = (struct multiboot_mmap_entry*) entry_addr;

        uint64_t end = entry->addr + entry->len;

        if (entry->type == MULTIBOOT_MMAP_AVAILABLE && end > max_addr)
            max_addr = end;

        entry_addr += entry->size + sizeof(entry->size);
    }

    // Limit the max address to 4 GiB.
    // Note: This is correct because sizeof(uintptr_t) < sizeof(uint64_t).
    if (max_addr > UINTPTR_MAX) {
        max_addr = UINTPTR_MAX + 1ULL;
    }

    // Use a shift here to avoid 64-bit division.
    return (size_t) (max_addr >> PAGE_OFFSET_BITS); // Round down to a number of pages.
}

// Set the state of a particular page in the bitmap
static void bitmap_set_allocated(size_t page, bool allocated) {
    assert(page < PMM_STATE.total_pages);
    if (allocated) {
        BITMAP[page / CHAR_BIT] |= 1 << (page % CHAR_BIT);
    } else {
        BITMAP[page / CHAR_BIT] &= ~(1 << (page % CHAR_BIT));
    }
}

// Check whether a particular page in the bitmap is allocated.
static bool bitmap_is_allocated(size_t page) {
    assert(page < PMM_STATE.total_pages);
    return (BITMAP[page / CHAR_BIT] >> (page % CHAR_BIT)) & 1;
}

// Set a region of pages as allocated or free.
// begin is inclusive, end is exclusive.
// TODO: Optimize
static void bitmap_mark_pages(size_t begin, size_t end , bool mark_as_allocated) {
    assert(begin <= end && end <= PMM_STATE.total_pages);

    for (size_t page = begin; page < end; ++page) {
        bitmap_set_allocated(page, mark_as_allocated);
    }
}

// Compute the initial state of the bitmap.
// This sets a bit corresponding to a page to 1 if it is already been allocated.
// This includes all pages which are not present in any memory region that is marked
// as available memory, and pages which are used to host the kernel itself.
// This function also frees parts of the bitmap which are unused.
// Returns the total number of free pages.
static size_t bitmap_init(const struct multiboot* mb, size_t pages, size_t bitmap_pages) {
    size_t free_pages = 0;

    // Mark the entire bitmap as allocated. Just handle this in page granularity, as
    // the size of the bitmap is rounded anyway.
    memset(BITMAP, 0xFF, bitmap_pages * PAGE_SIZE);

    // Mark multiboot available memory as free
    uintptr_t entry_addr = (uintptr_t) mb->mmap_addr;
    uintptr_t entry_end = (uintptr_t) mb->mmap_addr + mb->mmap_length;
    while (entry_addr < entry_end) {
        const struct multiboot_mmap_entry* entry = (struct multiboot_mmap_entry*) entry_addr;

        if (entry->type == MULTIBOOT_MMAP_AVAILABLE) {
            // Round first page up and last page down.
            uint64_t begin_page = PAGE_INDEX(PAGE_ALIGN_FORWARD(entry->addr));
            uint64_t end_page = PAGE_INDEX(entry->addr + entry->len);

            // Check that first page is inside 32-bit address range.
            if (begin_page < MAX_PAGES) {

                // Limit to 32-bit address range.
                // Note that after this point, the value is guaranteed to be small enough for 32-bit variables.
                if (end_page > MAX_PAGES)
                    end_page = MAX_PAGES;

                bitmap_mark_pages((size_t) begin_page, (size_t) end_page, false);
                free_pages += (size_t) (end_page - begin_page);
            }
        }

        entry_addr += entry->size + sizeof(entry->size);
    }

    // Mark the kernel area as allocated.
    uintptr_t kernel_begin_page = PAGE_INDEX(KERNEL_PHYSICAL_START);
    uintptr_t kernel_end_page = PAGE_INDEX(PAGE_ALIGN_FORWARD(KERNEL_PHYSICAL_END));
    bitmap_mark_pages(kernel_begin_page, kernel_end_page, true);
    free_pages -= (size_t) (kernel_end_page - kernel_begin_page);

    // Mark the unused part of the bitmap as free.
    uintptr_t bitmap_physical_start = (uintptr_t) KERNEL_VIRTUAL_TO_PHYSICAL(BITMAP);
    uintptr_t bitmap_free_begin_page = PAGE_INDEX(bitmap_physical_start) + bitmap_pages;
    uintptr_t bitmap_free_end_page = PAGE_INDEX(bitmap_physical_start) + MAX_BITMAP_PAGES;
    bitmap_mark_pages(bitmap_free_begin_page, bitmap_free_end_page, false);
    free_pages += (size_t) (bitmap_free_begin_page - bitmap_free_end_page);

    return free_pages;
}

// Note: in this function, we need to map the memory required by the physical
// memory manager manually, as there is no virtual memory manager available yet.
void pmm_init(const struct multiboot* mb) {
    log_info("Initializing physical memory manager");

    size_t pages = compute_total_pages(mb);

    // Compute the number of pages required to store the bitmap.
    // Note that we have to round up here.
    // Shifts are used here to avoid 64-bit division.
    size_t bitmap_bytes = (pages + CHAR_BIT - 1) >> CHAR_BIT_LOG2;
    size_t bitmap_pages = (bitmap_bytes + PAGE_SIZE - 1) >> PAGE_OFFSET_BITS;
    log_info("%zu system page(s), requires %zu bitmap page(s)", pages, bitmap_pages);

    assert(bitmap_pages <= MAX_BITMAP_PAGES);

    PMM_STATE.total_pages = pages;
    PMM_STATE.page_stack_top = 0;
    PMM_STATE.free_pages = bitmap_init(mb, pages, bitmap_pages);

    log_info("%zu/%zu physical page(s) free for allocation", pmm_free_pages(), pmm_total_pages());
}

size_t pmm_free_pages(void) {
    return PMM_STATE.free_pages;
}

size_t pmm_total_pages(void) {
    return PMM_STATE.total_pages;
}

void* pmm_alloc_page(void) {
    return NULL;
}

void pmm_free_page(void* page) {

}

