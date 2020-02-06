#include "core/multiboot.h"

#define MULTIBOOT_MAGIC_NUMBER (0x1BADB002)
#define MULTIBOOT_FLAGS (MULTIBOOT_REQUIRE_PAGE_ALIGN | MULTIBOOT_REQUIRE_MEMINFO)

const multiboot_header header __attribute__((section("multiboot"))) = {
    .magic = MULTIBOOT_MAGIC_NUMBER,
    .flags = MULTIBOOT_FLAGS,
    .checksum = -(MULTIBOOT_MAGIC_NUMBER + MULTIBOOT_FLAGS)
};
