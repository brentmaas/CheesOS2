#ifndef _CHEESOS2_MEMORY_VMM_H
#define _CHEESOS2_MEMORY_VMM_H

#include "memory/page_table.h"
#include "memory/page_range.h"

#include "utility/containers/rbtree.h"

#include <stdbool.h>

struct vmm_mapping {
    struct rb_node node;
    struct page_range range;
};

struct vmm_addrspace {
    struct page_directory* directory;
    struct rb_tree mappings;
};

void vmm_init(void);

struct vmm_addrspace* vmm_kernel_addrspace();

bool vmm_is_kernel_addrspace(const struct vmm_addrspace* addrspace);

struct page_range vmm_addrspace_range(const struct vmm_addrspace* addrspace);

void vmm_addrspace_dump_mappings(struct vmm_addrspace* addrspace);

#endif
