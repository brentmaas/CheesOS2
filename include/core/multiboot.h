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

typedef struct __attribute__((packed)) {
    uint32_t flags;

    void* mem_lower;
    void* mem_upper;

    void* boot_device;

    const char* cmdline;

    uint32_t mods_count;
    void* mods_addr;

    union {
        struct {
            uint32_t tabsize;
            uint32_t strsize;
            void* addr;
            uint32_t reserved;
        } a_out;
        struct {
            uint32_t num;
            uint32_t size;
            void* addr;
            uint32_t shndx;
        } elf;
    };



    uint32_t mmap_length;
    void* mmap_addr;

    uint32_t drives_length;
    void* drives_addr;

    void* config_table;

    const char* boot_loader_name;

    void* apm_table;

    void* vbe_control_info;
    void* vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;

    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t framebuffer_bpp;
    uint8_t framebuffer_type;

    union {
        struct {
            void* palette_addr;
            uint32_t palette_num_colors;
        } vbe_indexed_color;
        struct {
            uint8_t red_field_position;
            uint8_t red_mask_size;
            uint8_t green_field_position;
            uint8_t green_mask_size;
            uint8_t blue_field_position;
            uint8_t blue_mask_size;
        } vbe_direct_rgb;
    };
} multiboot_info;

typedef enum {
    MULTIBOOT_FLAG_MEM_COUNT = 1,
    MULTIBOOT_FLAG_BOOT_DEVICE = 2,
    MULTIBOOT_FLAG_CMDLINE = 4,
    MULTIBOOT_FLAG_MODULES = 8,
    MULTIBOOT_FLAG_EXECUTABLE_SYMBOL_A_OUT = 16,
    MULTIBOOT_FLAG_EXECUTE_SYMBOL_ELF = 32,
    MULTIBOOT_FLAG_MEMORY_MAP = 64,
    MULTIBOOT_FLAG_DRIVES = 128,
    MULTIBOOT_FLAG_BIOS_CONFIG_TABLE = 256,
    MULTIBOOT_FLAG_BOOT_LOADER_NAME = 512,
    MULTIBOOT_FLAG_APM_TABLE = 1024,
    MULTIBOOT_FLAG_VBE_INFO = 2048,
    MULTIBOOT_FLAG_FRAMEBUFFER_INFO = 4096
} multiboot_flags;

#endif
