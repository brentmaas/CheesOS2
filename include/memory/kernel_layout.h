#ifndef _CHEESOS2_MEMORY_KERNEL_LAYOUT_H
#define _CHEESOS2_MEMORY_KERNEL_LAYOUT_H

#include <stdint.h>

extern void* kernel_virtual_start;
extern void* kernel_start;
extern void* kernel_end;

inline uintptr_t kernel_virtual_start_addr() {
    return (uintptr_t) &kernel_virtual_start;
}

inline uintptr_t kernel_start_addr() {
    return (uintptr_t) &kernel_start;
}

inline uintptr_t kernel_end_addr() {
    return (uintptr_t) &kernel_end;
}

// Returns valid pointers only for things contained in the kernel image
// Any other virtual addresses added are invalid
#define KERNEL_VIRTUAL_TO_PHYSICAL(ptr) ((void*) (uintptr_t) (ptr) - (uintptr_t) &kernel_virtual_start)
#define KERNEL_PHYSICAL_TO_VIRTUAL(ptr) ((void*) (uintptr_t) (ptr) + (uintptr_t) &kernel_virtual_start)

#endif
