#include "core/panic.h"

#include <stdbool.h>

noreturn void kernel_panic(void) {
    while (true) {
        asm volatile (
            "cli\n"
            "hlt\n"
        );
    }

    // Cannot use `unreachable()` from debug/assert.h
    // kernel_panic is called by this.
    __builtin_unreachable();
}
