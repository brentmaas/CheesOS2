#ifndef _CHEESOS2_MEMORY_KERNEL_LAYOUT_H
#define _CHEESOS2_MEMORY_KERNEL_LAYOUT_H

#include <stdint.h>

extern void* kernel_virtual_start;
extern void* kernel_virtual_end;
extern void* kernel_physical_start;
extern void* kernel_physical_end;

#define KERNEL_VIRTUAL_START ((uintptr_t) &kernel_virtual_start)
#define KERNEL_VIRTUAL_END ((uintptr_t) &kernel_virtual_end)

#define KERNEL_PHYSICAL_START ((uintptr_t) &kernel_physical_start)
#define KERNEL_PHYSICAL_END ((uintptr_t) &kernel_physical_end)

// Returns valid pointers only for things contained in the kernel image
// Any other virtual addresses added are invalid
#define KERNEL_VIRTUAL_TO_PHYSICAL(ptr) ((void*) (uintptr_t) (ptr) - KERNEL_VIRTUAL_START)
#define KERNEL_PHYSICAL_TO_VIRTUAL(ptr) ((void*) (uintptr_t) (ptr) + KERNEL_VIRTUAL_START)

#endif
