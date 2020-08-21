#ifndef _CHEESOS2_UTILITY_CPRINTF_H
#define _CHEESOS2_UTILITY_CPRINTF_H

#include <stddef.h>
#include <stdarg.h>

#define FORMAT_ERR_INVALID_SPECIFIER (-1)

typedef int (*cprintf_write_cbk)(void* ctx, const char* data, size_t size);

int vcprintf(cprintf_write_cbk cbk, void* ctx, const char* format, va_list args);
int cprintf(cprintf_write_cbk cbk, void* ctx, const char* format, ...);

int vsnprintf(char* restrict buffer, size_t bufsz, const char* restrict format, va_list args);
int snprintf(char* restrict buffer, size_t bufsz, const char* restrict format, ...);

int vbprintf(char** restrict buffer, size_t* size_left, const char* restrict format, va_list args);
int bprintf(char** restrict buffer, size_t* size_left, const char* restrict format, ...);

#endif
