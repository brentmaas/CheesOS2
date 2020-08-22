#include "core/panic.h"

noreturn void kernel_panic(void) {
    asm volatile (
        "cli\n"
        "hlt\n"
    );

    // Cannot use `unreachable()` from debug/assert.h
    // kernel_panic is called by this.
    __builtin_unreachable();
}
