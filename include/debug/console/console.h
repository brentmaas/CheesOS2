#ifndef _CHEESOS2_DEBUG_CONSOLE_CONSOLE_H
#define _CHEESOS2_DEBUG_CONSOLE_CONSOLE_H

#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>

#include "debug/log.h"

void console_init(void);
void console_clear(void);
void console_set_attr(uint8_t fg, uint8_t bg);
void console_putchar(char c);
void console_erasechar(void);
void console_write(const char* data, size_t size);
void console_print(const char* str);
void console_vprintf(const char* format, va_list args);
void console_printf(const char* format, ...);
void console_scroll(uint8_t rows);
void console_log_sink(void*, enum log_level level, const char* file, unsigned line, const char* format, va_list args);

#endif
