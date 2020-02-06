#include <string.h>

size_t strlen(const char* str) {
    size_t count = 0;
    while(*str++) {
        ++count;
    }
    return count;
}