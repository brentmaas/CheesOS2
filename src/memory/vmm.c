#include "memory/vmm.h"
#include "memory/pmm.h"
#include "memory/kernel_layout.h"

#include "debug/log.h"
#include "debug/assert.h"

#include "utility/container_of.h"

#include <stdbool.h>

static struct page_directory VMM_KERNEL_PAGE_DIR;
static struct page_table VMM_KERNEL_PAGE_TABLE;
static struct vmm_addrspace VMM_KERNEL_ADDRESS_SPACE;
static struct vmm_mapping VMM_KERNEL_MAPPING;

__attribute__((section(".bootstrap.text")))
struct page_directory* vmm_bootstrap(void) {
    // Get the physical address of the page dir and kernel page table
    struct page_directory* pd = KERNEL_VIRTUAL_TO_PHYSICAL(&VMM_KERNEL_PAGE_DIR);
    struct page_table* pt = KERNEL_VIRTUAL_TO_PHYSICAL(&VMM_KERNEL_PAGE_TABLE);

    for (size_t i = 0; i < PAGE_DIR_ENTRY_COUNT; ++i) {
        pd->entries[i] = (struct page_dir_entry){};
    }

    for (size_t i = 0; i < PAGE_TABLE_ENTRY_COUNT; ++i) {
        pt->entries[i] = (struct page_table_entry){};
    }

    // Identity map to prevent generating page faults when enabling paging.
    // To be removed after.
    pd->entries[0] = (struct page_dir_entry){
        .present = true,
        .write_enable = true,
        .page_table_address = PAGE_ADDR((uintptr_t) pt)
    };

    // The actual kernel page table.
    // TODO: The kernel virtual and physical starts don't quite make sense. This should
    // probably start at 0xC0100000 instead of 0xC0000000.
    pd->entries[PAGE_DIR_INDEX(KERNEL_VIRTUAL_START)] = (struct page_dir_entry){
        .present = true,
        .write_enable = true,
        .page_table_address = PAGE_ADDR((uintptr_t) pt)
    };

    uintptr_t kernel_end_page = PAGE_ADDR(PAGE_ALIGN_FORWARD(KERNEL_PHYSICAL_END));

    // Add everything up to kernel_end_addr to the kernel page table
    // kernel.ld guarantees that kernel_end_page fits in the page table
    for (size_t i = 0; i < kernel_end_page; ++i) {
        pt->entries[i] = (struct page_table_entry){
            .present = true,
            .write_enable = true,
            .user = true,
            .page_address = i
        };
    }

    return pd;
}

static int vmm_mapping_cmp_base(struct rb_node* lhs, struct rb_node* rhs) {
    return page_range_cmp_base(
        &CONTAINER_OF(struct vmm_mapping, node, lhs)->range,
        &CONTAINER_OF(struct vmm_mapping, node, rhs)->range
    );
}

void vmm_init(void) {
    // Initialize the kernel address space
    struct vmm_addrspace* addrspace = &VMM_KERNEL_ADDRESS_SPACE;

    addrspace->directory = &VMM_KERNEL_PAGE_DIR;
    rb_init(&addrspace->mappings, vmm_mapping_cmp_base);

    // TODO: The kernel virtual and physical starts don't quite make sense.
    struct vmm_mapping* kernel_mapping = &VMM_KERNEL_MAPPING;
    kernel_mapping->range.base = PAGE_ADDR((uintptr_t) KERNEL_PHYSICAL_TO_VIRTUAL(KERNEL_PHYSICAL_START));
    kernel_mapping->range.size = PAGE_ADDR(PAGE_ALIGN_FORWARD(KERNEL_PHYSICAL_END) - KERNEL_PHYSICAL_START);

    rb_insert(&addrspace->mappings, &kernel_mapping->node);

    // Clear the identity map
    VMM_KERNEL_PAGE_DIR.entries[0] = (struct page_dir_entry){};
    pt_invalidate_tlb();
}

struct vmm_addrspace* vmm_kernel_addrspace() {
    return &VMM_KERNEL_ADDRESS_SPACE;
}

bool vmm_is_kernel_addrspace(const struct vmm_addrspace* addrspace) {
    return addrspace == vmm_kernel_addrspace();
}

struct page_range vmm_addrspace_range(const struct vmm_addrspace* addrspace) {
    if (vmm_is_kernel_addrspace(addrspace)) {
        // Kernel address space is from virtual start to the end.
        // Note: Unsigned overflow here is expected.
        return (struct page_range){.base = PAGE_ADDR(KERNEL_VIRTUAL_START), .size = PAGE_ADDR(-KERNEL_VIRTUAL_START)};
    } else {
        // User address space is from 0 to kernel virtual start.
        return (struct page_range){.base = 0, .size = PAGE_ADDR(KERNEL_VIRTUAL_START)};
    }
}

void vmm_addrspace_dump_mappings(struct vmm_addrspace* addrspace) {
    struct page_range range = vmm_addrspace_range(addrspace);
    log_debug(
        "Address space range: %p, %u KiB (%zu pages)",
        (void*) (range.base * PAGE_SIZE),
        range.size * (PAGE_SIZE / 1024),
        range.size
    );

    log_debug("Mappings:");

    struct rb_node* node = rb_first_node(&addrspace->mappings);
    while (node) {
        struct vmm_mapping* mapping = CONTAINER_OF(struct vmm_mapping, node, node);
        log_debug(
            "%p, %u KiB (%zu pages)",
            (void*) (mapping->range.base * PAGE_SIZE),
            mapping->range.size * (PAGE_SIZE / 1024),
            mapping->range.size
        );

        node = rb_next_node(node);
    }
}
