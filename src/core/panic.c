#include "core/panic.h"

void kernel_panic(void) {
    asm volatile (
        "cli\n"
        "hlt\n"
    );
}