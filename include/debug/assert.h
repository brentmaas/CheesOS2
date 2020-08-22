#ifndef _CHEESOS2_DEBUG_ASSERT_H
#define _CHEESOS2_DEBUG_ASSERT_H

#include <stdnoreturn.h>

// TODO: Release/debug mode

#define assert(cond) ((cond) ? (void) 0 : assert_report((#cond), __FILE__, __PRETTY_FUNCTION__, __LINE__))
#define unreachable() assert_report_unreachable(__FILE__, __PRETTY_FUNCTION__, __LINE__)

noreturn void assert_report(const char* expr, const char* file, const char* function, unsigned line);
noreturn void assert_report_unreachable(const char* file, const char* function, unsigned line);

#endif
