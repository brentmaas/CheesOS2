#ifndef _CHEESOS2_INCLUDE_LIBC_STRING_H
#define _CHEESOS2_INCLUDE_LIBC_STRING_H

#include <stddef.h>

size_t strlen(const char*);

void* memset(void* dest, int ch, size_t count);
void* memcpy(void* restrict dest, const void* restrict src, size_t count);
void* memmove(void* dest, const void* src, size_t count);
int memcmp(const void* lhs, const void* rhs, size_t count);
void* memchr(const void* ptr, int ch, size_t count);

#endif
