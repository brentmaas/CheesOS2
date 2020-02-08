#ifndef _CHEESOS2_INCLUDE_LIBC_STDIO_H
#define _CHEESOS2_INCLUDE_LIBC_STDIO_H

#include <stddef.h>

size_t printf(const char* format, ...);
size_t snprintf(char* buffer, size_t buffer_size, const char* format, ...);
size_t sprinft(char* buffer, const char* format, ...);

#endif