#include "debug/log.h"

#include <stddef.h>
#include <stdarg.h>

static log_sink LOG_SINK = NULL;
static void* LOG_CONTEXT = NULL;

const char* LOG_LEVEL_NAMES[] = {
    [LOG_LEVEL_DEBUG] = "DEBUG",
    [LOG_LEVEL_INFO] = "INFO",
    [LOG_LEVEL_WARN] = "WARN",
    [LOG_LEVEL_ERROR] = "ERROR"
};

void log_set_sink(log_sink sink, void* context) {
    LOG_SINK = sink;
    LOG_CONTEXT = context;
}

void log_write(enum log_level level, const char* file, unsigned line, const char* format, ...) {
    if (LOG_SINK != NULL) {
        va_list args;
        va_start(args, format);
        LOG_SINK(LOG_CONTEXT, level, file, line, format, args);
        va_end(args);
    }
}

