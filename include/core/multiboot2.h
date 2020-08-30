#ifndef _CHEESOS2_CORE_MULTIBOOT2_H
#define _CHEESOS2_CORE_MULTIBOOT2_H

#include <stdint.h>
#include <stddef.h>

#include "memory/align.h"

#define MULTIBOOT2_HEADER_MAGIC (0xe85250d6)

enum multiboot2_arch {
    MULTIBOOT2_ARCH_I386 = 0,
    MULTIBOOT2_ARCH_MIPS32 = 4
};

struct multiboot2_header {
    uint32_t magic;
    enum multiboot2_arch architecture;
    uint32_t header_length;
    uint32_t checksum;
};

enum multiboot2_header_tag_type {
    MULTIBOOT2_HEADER_TAG_TYPE_END = 0,
    MULTIBOOT2_HEADER_TAG_TYPE_INFORMATION_REQUEST = 1,
    MULTIBOOT2_HEADER_TAG_TYPE_ADDRESS = 2,
    MULTIBOOT2_HEADER_TAG_TYPE_ENTRY_ADDRESS = 3,
    MULTIBOOT2_HEADER_TAG_TYPE_CONSOLE_FLAGS = 4,
    MULTIBOOT2_HEADER_TAG_TYPE_FRAMEBUFFER = 5,
    MULTIBOOT2_HEADER_TAG_TYPE_MODULE_ALIGN = 6,
    MULTIBOOT2_HEADER_TAG_TYPE_EFI_BS = 7,
    MULTIBOOT2_HEADER_TAG_TYPE_ENTRY_ADDRESS_EFI32 = 8,
    MULTIBOOT2_HEADER_TAG_TYPE_ENTRY_ADDRESS_EFI64 = 9,
    MULTIBOOT2_HEADER_TAG_TYPE_RELOCATABLE = 10
};

struct multiboot2_header_tag {
    uint16_t type;
    uint16_t optional;
    uint32_t size;
};

struct multiboot2_header_tag_information_request {
    struct multiboot2_header_tag tag;
    enum multiboot2_header_tag_type requests[0]; // (size - 4) / 4 elements
};

struct multiboot2_header_tag_address {
    struct multiboot2_header_tag tag;
    uint32_t header_addr;
    uint32_t load_addr;
    uint32_t load_end_addr;
    uint32_t bss_end_addr;
};

struct multiboot2_header_tag_entry_address {
    struct multiboot2_header_tag tag;
    uint32_t entry_addr;
};

enum multiboot2_console_flags {
    MULTIBOOT2_CONSOLE_REQUIRED_BIT = 1 << 0,
    MULTIBOOT2_CONSOLE_EGA_TEXT_BIT = 1 << 1
};

struct multiboot2_header_tag_console_flags {
    struct multiboot2_header_tag tag;
    enum multiboot2_console_flags console_flags;
};

struct multiboot2_header_tag_framebuffer {
    struct multiboot2_header_tag tag;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
};

struct multiboot2_header_tag_module_align {
    struct multiboot2_header_tag tag;
};

enum multiboot2_load_preference {
    MULTIBOOT2_LOAD_PREFERENCE_NONE = 0,
    MULTIBOOT2_LOAD_PREFERENCE_LOW = 1,
    MULTIBOOT2_LOAD_PREFERENCE_HIGH = 2
};

struct multiboot2_header_tag_efi_bs {
    struct multiboot2_header_tag tag;
};

struct multiboot2_header_tag_entry_address_efi32 {
    struct multiboot2_header_tag tag;
    uint32_t entry_addr;
};

struct multiboot2_header_tag_entry_address_efi64 {
    struct multiboot2_header_tag tag;
    uint32_t entry_addr;
};

struct multiboot2_header_tag_relocatable {
    struct multiboot2_header_tag tag;

    uint32_t min_addr;
    uint32_t max_addr;
    uint32_t align;
    enum multiboot2_load_preference load_preference;
};

enum multiboot2_tag_type {
    MULTIBOOT2_TAG_TYPE_END = 0,
    MULTIBOOT2_TAG_TYPE_CMDLINE = 1,
    MULTIBOOT2_TAG_TYPE_BOOT_LOADER_NAME = 2,
    MULTIBOOT2_TAG_TYPE_MODULE = 3,
    MULTIBOOT2_TAG_TYPE_BASIC_MEMINFO = 4,
    MULTIBOOT2_TAG_TYPE_BOOTDEV = 5,
    MULTIBOOT2_TAG_TYPE_MMAP = 6,
    MULTIBOOT2_TAG_TYPE_VBE = 7,
    MULTIBOOT2_TAG_TYPE_FRAMEBUFFER = 8,
    MULTIBOOT2_TAG_TYPE_ELF_SECTIONS = 9,
    MULTIBOOT2_TAG_TYPE_APM = 10,
    MULTIBOOT2_TAG_TYPE_EFI32 = 11,
    MULTIBOOT2_TAG_TYPE_EFI64 = 12,
    MULTIBOOT2_TAG_TYPE_SMBIOS = 13,
    MULTIBOOT2_TAG_TYPE_ACPI_OLD = 14,
    MULTIBOOT2_TAG_TYPE_ACPI_NEW = 15,
    MULTIBOOT2_TAG_TYPE_NETWORK = 16,
    MULTIBOOT2_TAG_TYPE_EFI_MMAP = 17,
    MULTIBOOT2_TAG_TYPE_EFI_BS = 18,
    MULTIBOOT2_TAG_TYPE_EFI32_IH = 19,
    MULTIBOOT2_TAG_TYPE_EFI64_IH = 20,
    MULTIBOOT2_TAG_TYPE_LOAD_BASE_ADDR = 21
};

struct multiboot2_info {
    uint32_t total_size;
    uint32_t _reserved;
};

struct multiboot2_tag {
    enum multiboot2_tag_type type;
    uint32_t size;
};

struct multiboot2_tag_cmdline {
    struct multiboot2_tag tag;
    uint8_t cmdline[0];
};

struct multiboot2_tag_boot_loader_name {
    struct multiboot2_tag tag;
    uint8_t boot_loader_name[0];
};

struct multiboot2_tag_basic_meminfo {
    struct multiboot2_tag tag;
    uint32_t mem_lower;
    uint32_t mem_upper;
};

enum multiboot2_mmap_type {
    MULTIBOOT2_MMAP_AVAILABLE = 1,
    MULTIBOOT2_MMAP_RESERVED = 2,
    MULTIBOOT2_MMAP_ACPI = 3,
    MULTIBOOT2_MMAP_ACPI_NVS = 4,
    MULTIBOOT2_MMAP_DEFECTIVE = 5
};

struct multiboot2_mmap_entry {
    uint64_t addr;
    uint64_t len;
    enum multiboot2_mmap_type type;
    uint32_t _zero;
};

struct multiboot2_tag_mmap {
    struct multiboot2_tag tag;
    uint32_t entry_size;
    uint32_t version;
    struct multiboot2_mmap_entry entries[0];
};

inline const struct multiboot2_tag* multiboot2_first_tag(const struct multiboot2_info* info) {
    return (void*) ((uintptr_t) info + sizeof(struct multiboot2_info));
}

inline const struct multiboot2_tag* multiboot2_next_tag(const struct multiboot2_tag* tag) {
    return (void*) ALIGN_FORWARD_2POW((uintptr_t) tag + tag->size, 8);
}

#endif
