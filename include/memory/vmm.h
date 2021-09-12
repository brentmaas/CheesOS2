#ifndef _CHEESOS2_MEMORY_VMM_H
#define _CHEESOS2_MEMORY_VMM_H

#include "memory/page_table.h"

extern struct page_directory VMM_ROOT_PAGE_DIR;
extern struct page_table VMM_KERNEL_PAGE_TABLE;

// Bootstrapping memory identity maps kernel memory. This function removes that mapping.
void vmm_unmap_identity();

#endif
