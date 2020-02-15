#include <stdlib.h>
#include <stdint.h>

size_t strtozu(const char* restrict str, const char** restrict str_end) {
    size_t value = 0;
    while (*str >= '0' && *str <= '9') {
        int digit = *str - '0';
        if (value > (SIZE_MAX - digit) / 10) {
            *str_end = NULL;
            return 0;
        }

        value *= 10;
        value += *str - '0';
        ++str;
    }

    *str_end = str;
    return value;
}
