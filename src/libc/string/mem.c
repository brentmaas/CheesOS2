#include <string.h>
#include <stdint.h>

void* memset(void* dest, int ch, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        ((uint8_t*) dest)[i] = ch;
    }

    return dest;
}

void* memcpy(void* restrict dest, const void* restrict src, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        ((uint8_t*) dest)[i] = ((uint8_t*) src)[i];
    }

    return dest;
}

void* memmove(void* dest, const void* src, size_t count) {
    if (dest < src) {
        for (size_t i = 0; i < count; ++i) {
            ((uint8_t*) dest)[i] = ((uint8_t*) src)[i];
        }
    } else if (dest > src) {
        while (count > 0) {
            --count;
            ((uint8_t*) dest)[count] = ((uint8_t*) src)[count];
        }
    }

    return dest;
}

int memcmp(const void* lhs, const void* rhs, size_t count) {
    const uint8_t* lhs_bytes = lhs;
    const uint8_t* rhs_bytes = rhs;
    for (size_t i = 0; i < count; ++i) {
        if (lhs_bytes[i] > rhs_bytes[i]) {
            return 1;
        } else if (lhs_bytes[i] < rhs_bytes[i]) {
            return -1;
        }
    }

    return 0;
}

void* memchr(const void* ptr, int ch, size_t count) {
    const uint8_t* bytes = ptr;
    for (size_t i = 0; i < count; ++i) {
        if (bytes[i] == ch) {
            return (void*) &bytes[i]; // cast away constness
        }
    }

    return NULL;
}
