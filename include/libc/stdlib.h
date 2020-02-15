#ifndef _CHEESOS2_LIBC_STDLIB_H
#define _CHEESOS2_LIBC_STDLIB_H

#include <stddef.h>

// Returns the end pointer in str_end, or nullptr if conversion failed due
// to out-of-range. If there was no integer, `*str_end == str` and 0 is returned
size_t strtozu(const char* restrict str, const char** restrict str_end);

#endif
