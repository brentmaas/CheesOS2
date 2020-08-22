#include "debug/assert.h"
#include "debug/log.h"
#include "core/panic.h"

#include <stdbool.h>

// TODO: Replace by lock
static bool reporting = false;

noreturn void assert_report(const char* expr, const char* file, const char* function, unsigned line) {
    if (!reporting) {
        reporting = true;
        log_error("Assertion `%s` failed at %s:%s:%u", expr, file, function, line);
        reporting = false;
    }
    kernel_panic();
}

noreturn void assert_report_unreachable(const char* file, const char* function, unsigned line) {
    if (!reporting) {
        reporting = true;
        log_error("Reached unreachable code at %s:%s:%u", file, function, line);
        reporting = false;
    }
    kernel_panic();
}
