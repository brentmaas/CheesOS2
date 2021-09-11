#ifndef _CHEESOS2_MEMORY_MEMORY_H
#define _CHEESOS2_MEMORY_MEMORY_H

#include "memory/page_table.h"
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

#endif
