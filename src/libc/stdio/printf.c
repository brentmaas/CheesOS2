#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "vga/vga.h"

#define UINTMAX_DIGITS 20 // log10(uintmax_t) == 20 digits
#define UINTMAX_NIBBLES (sizeof(uintmax_t) * 2)

static size_t format_uint(uintmax_t value) {
    _Static_assert(sizeof(uintmax_t) == sizeof(uint64_t));
    char buf[UINTMAX_DIGITS] = {0};
    size_t digit = UINTMAX_DIGITS;

    do {
        uint64_t rem;
        value = udivmod64(value, 10, &rem);
        buf[--digit] = '0' + (char) rem;
    } while (value > 0);

    vga_write(&buf[digit], UINTMAX_DIGITS - digit);
    return UINTMAX_DIGITS - digit;
}

static size_t format_hex(uintmax_t value, bool upper) {
    char buf[UINTMAX_NIBBLES] = {0};
    char letter_base = (upper ? 'A' : 'a') - 10;

    size_t digit = UINTMAX_NIBBLES;
    do {
        char nibble = (char)(value & 0xF);
        buf[--digit] = nibble + (nibble < 10 ? '0' : letter_base);
        value >>= 4;
    } while (value > 0);

    vga_write(&buf[digit], UINTMAX_NIBBLES - digit);
    return UINTMAX_NIBBLES - digit;
}

size_t printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    size_t printed = 0;
    while (*format) {
        if (*format == '%') {
            ++format;
            if (*format == '%') {
                vga_putchar('%');
                ++printed;
            } else if (*format == 'i' || *format == 'd') {
                int i = va_arg(args, int);
                if (i < 0) {
                    vga_putchar('-');
                    printed += format_uint(~((uintmax_t) i) + 1);
                } else {
                    printed += format_uint(i);
                }
            } else if (*format == 'z') {
                size_t i = va_arg(args, size_t);
                printed += format_uint(i);
            } else if (*format == 'j') {
                uintmax_t i = va_arg(args, uintmax_t);
                printed += format_uint(i);
            } else if (*format == 'u') {
                unsigned i = va_arg(args, unsigned);
                printed += format_uint(i);
            } else if (*format == 's') {
                char* s = va_arg(args, char*);
                vga_print(s);
                printed += strlen(s);
            } else if (*format == 'c') {
                int c = va_arg(args, int);
                vga_putchar((char) c);
                ++printed;
            } else if (*format == 'x' || *format == 'X') {
                unsigned i = va_arg(args, unsigned);
                printed += format_hex(i, *format == 'X');
            } else {
                return SIZE_MAX;
            }
        } else {
            vga_putchar(*format);
            ++printed;
        }

        ++format;
    }

    va_end(args);
    return printed;
}
