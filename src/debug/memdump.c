#include "debug/memdump.h"
#include "debug/log.h"
#include "utility/cprintf.h"

#include <stdint.h>

void debug_memdump(void* addr, size_t size) {
    uint8_t* data = addr;
    for (size_t i = 0; i < size; i += 16) {
        char buffer[16 * 3];
        char* ptr = buffer;
        size_t size_left = sizeof(buffer);

        size_t end = size - i < 16 ? size - i : 16;
        for (size_t j = 0; j < end; j += 1) {
            bprintf(&ptr, &size_left, "%02X ", data[j + i]);
        }

        log_debug(buffer);
    }
}
