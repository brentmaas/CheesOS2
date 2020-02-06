#ifndef _CHEESOS2_CORE_MULTIBOOT_H
#define _CHEESOS2_CORE_MULTIBOOT_H

#include <stdint.h>

typedef struct {
    uint32_t magic;
    uint32_t flags;
    uint32_t checksum;
} multiboot_header;

enum __multiboot_header_flags {
    MULTIBOOT_REQUIRE_PAGE_ALIGN = 1 << 0,
    MULTIBOOT_REQUIRE_MEMINFO = 1 << 1,
    MULTIBOOT_REQUIRE_VIDEOINFO = 1 << 2,
    MULTIBOOT_REQUIRE_CUSTOM_LOAD = 1 << 16
};

#endif
