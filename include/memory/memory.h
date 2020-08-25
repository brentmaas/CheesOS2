#ifndef _CHEESOS2_MEMORY_MEMORY_H
#define _CHEESOS2_MEMORY_MEMORY_H

#include "memory/paging.h"
#include "core/multiboot.h"

#include <stddef.h>

enum memory_result {
    MEMORY_OK,
    MEMORY_NO_VALID_ZONE,
    MEMORY_UNMAPPED_VADDR,
    MEMORY_PHYSICAL_ALLOC_FAILED,
};

enum memory_map_flags {
    MEMORY_MAP_WRITABLE = 0x1,
    MEMORY_MAP_USER = 0x2,
};

// After this returns, any memory which might be in the > 1MiB region
// (such as the multiboot information structure) might be invalid.
// Returns 0 on success
enum memory_result memory_init(const struct multiboot_info* multiboot);

void* memory_alloc_physical(size_t pages);
void memory_free_physical(void* memory);

// Returns 0 on success, -1 when failed to allocate a page table.
enum memory_result memory_set_mapping(struct page_directory* pdir, void* virtual, void* physical, enum memory_map_flags);

// Returns 0 on success, -1 when failed to allocate a page or page table.
enum memory_result memory_map(struct page_directory* pdir, void* virtual, enum memory_map_flags);

// Returns 0 on success, -1 when `virtual` is not a valid mapping.
enum memory_result memory_unmap(struct page_directory* pdir, void* virtual);

// Returns 0 on success, -1 when `virtual` is not a valid mapping.
enum memory_result memory_translate(const struct page_directory* pdir, void* virtual, void** physical);

#endif
