#ifndef _CHEESOS2_UTILITY_CPRINTF_H
#define _CHEESOS2_UTILITY_CPRINTF_H

#include <stddef.h>
#include <stdarg.h>

#define FORMAT_ERR_INVALID_SPECIFIER (-1)

typedef int (*cprintf_write_cbk)(void* ctx, const char* data, size_t size);

int vcprintf(cprintf_write_cbk cbk, void* ctx, const char* format, va_list args);
int cprintf(cprintf_write_cbk cbk, void* ctx, const char* format, ...);

#endif
