#ifndef _CHEESOS2_CORE_BOOT_INFO_H
#define _CHEESOS2_CORE_BOOT_INFO_H

#include <stdint.h>
#include <stddef.h>

struct boot_info_mmap_entry {
    uint64_t addr;
    uint64_t len;
};

struct boot_info {
    const char* cmdline;
    const char* boot_loader_name;

    size_t mmap_len;
    const struct boot_info_mmap_entry* mmap;
};

#endif
