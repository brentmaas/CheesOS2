#ifndef _CHEESOS2_MEMORY_PAGING_H
#define _CHEESOS2_MEMORY_PAGING_H

#include <stdint.h>

#define PAGE_SIZE (0x1000)
#define PAGE_DIR_ENTRY_COUNT (1024)
#define PAGE_TABLE_ENTRY_COUNT (1024)

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

struct __attribute__((packed)) page_directory {
    struct page_dir_entry entries[PAGE_DIR_ENTRY_COUNT];
};

struct __attribute__((packed)) page_table {
    struct page_table_entry entries[PAGE_TABLE_ENTRY_COUNT];
};

extern void paging_enable(void);
extern void paging_disable(void);
extern void paging_invalidate_tlb(void);
extern void paging_invalidate_tlb_entry(void* addr);
extern void paging_load_directory(struct page_directory* dir);

#endif
