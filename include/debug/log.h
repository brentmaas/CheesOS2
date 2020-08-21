#ifndef _CHEESOS2_DEBUG_LOG_H
#define _CHEESOS2_DEBUG_LOG_H

#include <stdarg.h>

#include "utility/cprintf.h"

enum log_level {
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR
};

extern const char* LOG_LEVEL_NAMES[];

typedef void (*log_sink)(void* context, enum log_level level, const char* file, unsigned line, const char* format, va_list args);

void log_set_sink(log_sink sink, void* context);
void log_write(enum log_level level, const char* file, unsigned line, const char* format, ...);

#define log_debug(...) log_write(LOG_LEVEL_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...) log_write(LOG_LEVEL_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...) log_write(LOG_LEVEL_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) log_write(LOG_LEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__)

#endif
