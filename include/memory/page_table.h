#ifndef _CHEESOS2_MEMORY_PAGE_TABLE_H
#define _CHEESOS2_MEMORY_PAGE_TABLE_H

#include "memory/align.h"

#include <stdint.h>
#include <stddef.h>

#define PAGE_DIR_ENTRY_BITS (10U)
#define PAGE_TABLE_ENTRY_BITS (10U)
#define PAGE_OFFSET_BITS (12U)

#define PAGE_DIR_ENTRY_COUNT (1U << PAGE_DIR_ENTRY_BITS)
#define PAGE_TABLE_ENTRY_COUNT (1U << PAGE_TABLE_ENTRY_BITS)
#define PAGE_SIZE (1U << PAGE_OFFSET_BITS)

#define PAGE_DIR_INDEX(addr) (((addr) >> (PAGE_OFFSET_BITS + PAGE_TABLE_ENTRY_BITS)) & (PAGE_DIR_ENTRY_COUNT - 1))
#define PAGE_TABLE_INDEX(addr) (((addr) >> PAGE_OFFSET_BITS) & (PAGE_TABLE_ENTRY_COUNT - 1))
#define PAGE_OFFSET(addr) ((addr) & (PAGE_SIZE - 1))

#define PAGE_ADDR(addr) ((addr) >> PAGE_OFFSET_BITS)

#define PAGE_ALIGN_BACKWARD(addr) (ALIGN_BACKWARD_2POW((addr), PAGE_SIZE))
#define PAGE_ALIGN_FORWARD(addr) (ALIGN_FORWARD_2POW((addr), PAGE_SIZE))

#define IS_PAGE_ALIGNED(addr) ((addr) % PAGE_SIZE == 0)

// A type which is guaranteed to be able to hold all page addresses plus one.
// This makes the type able to also address the page past the last page.
typedef uintptr_t pageaddr_t;

struct __attribute__((packed)) page_dir_entry {
    uint8_t present : 1;
    uint8_t write_enable : 1;
    uint8_t user : 1;
    uint8_t write_through : 1;
    uint8_t cache_disable : 1;
    uint8_t accessed : 1;
    uint8_t ignored0 : 1;
    uint8_t is_huge_page : 1;
    uint8_t ignored1 : 4;
    pageaddr_t page_table_address : 20;
};

struct __attribute__((packed)) page_table_entry {
    uint8_t present : 1;
    uint8_t write_enable : 1;
    uint8_t user : 1;
    uint8_t write_through : 1;
    uint8_t cache_disable : 1;
    uint8_t accessed : 1;
    uint8_t dirty : 1;
    uint8_t pat : 1;
    uint8_t global : 1;
    uint8_t ignored : 3;
    pageaddr_t page_address : 20;
};

struct __attribute__((packed, aligned(PAGE_SIZE))) page_directory {
    struct page_dir_entry entries[PAGE_DIR_ENTRY_COUNT];
};

struct __attribute__((packed, aligned(PAGE_SIZE))) page_table {
    struct page_table_entry entries[PAGE_TABLE_ENTRY_COUNT];
};

extern void pt_invalidate_tlb(void);
extern void pt_invalidate_address(void* addr);
extern void pt_load_directory(struct page_directory* pd);

#endif
