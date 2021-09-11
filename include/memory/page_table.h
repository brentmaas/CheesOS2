#ifndef _CHEESOS2_MEMORY_PAGE_TABLE_H
#define _CHEESOS2_MEMORY_PAGE_TABLE_H

#include <stdint.h>

#include "memory/align.h"

#define PAGE_DIR_ENTRY_BITS (10u)
#define PAGE_TABLE_ENTRY_BITS (10u)
#define PAGE_OFFSET_BITS (12u)

#define PAGE_DIR_ENTRY_COUNT (1 << PAGE_DIR_ENTRY_BITS)
#define PAGE_TABLE_ENTRY_COUNT (1 << PAGE_TABLE_ENTRY_BITS)
#define PAGE_SIZE (1 << PAGE_OFFSET_BITS)

#define PAGE_DIR_INDEX(addr) (((addr) >> (PAGE_OFFSET_BITS + PAGE_TABLE_ENTRY_BITS)) & (PAGE_DIR_ENTRY_COUNT - 1))
#define PAGE_TABLE_INDEX(addr) (((addr) >> PAGE_OFFSET_BITS) & (PAGE_TABLE_ENTRY_COUNT - 1))
#define PAGE_OFFSET(addr) ((addr) & (PAGE_SIZE - 1))

#define PAGE_ALIGN_BACKWARD(addr) (ALIGN_BACKWARD_2POW((addr), PAGE_SIZE))
#define PAGE_ALIGN_FORWARD(addr) (ALIGN_FORWARD_2POW((addr), PAGE_SIZE))

#define IS_PAGE_ALIGNED(addr) ((addr) % PAGE_SIZE == 0)

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
    uint32_t page_table_address : 20;
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
    uint32_t page_address : 20;
};

struct __attribute__((packed, aligned(PAGE_SIZE))) page_directory {
    struct page_dir_entry entries[PAGE_DIR_ENTRY_COUNT];
};

struct __attribute__((packed, aligned(PAGE_SIZE))) page_table {
    struct page_table_entry entries[PAGE_TABLE_ENTRY_COUNT];
};

extern void pt_invalidate_tlb(void);
extern void pt_invalidate_address(void* addr);
extern void pt_load_directory(struct page_directory* dir);

#endif
