#ifndef _CHEESOS2_INCLUDE_LIBC_STDIO_H
#define _CHEESOS2_INCLUDE_LIBC_STDIO_H

#include <stddef.h>
#include <stdarg.h>

typedef int (*print_cbk)(void* context, const char* data, size_t size);

int vcprintf(print_cbk cbk, void* context, const char* format, va_list args);
int cprintf(print_cbk cbk, void* context, const char* format, ...);

void printf(const char* format, ...);

#endif