#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "vga/vga.h"

#define UINTMAX_DIGITS 20 // log10(uintmax_t) == 20 digits
#define UINTMAX_NIBBLES (sizeof(uintmax_t) * 2)

enum format_length {
    LENGTH_CHAR,
    LENGTH_SHORT,
    LENGTH_INT,
    LENGTH_LONG,
    LENGTH_LONG_LONG,
    LENGTH_SIZE,
    LENGTH_UMAX
};

struct format_options {
    struct {
        bool leading_zeros;
    } flags;

    size_t min_width;
    enum format_length length_modifier;
    char conversion_specifier;
};

static size_t write_uint_buf(const struct format_options* opts, const char* buf, size_t len) {
    char leading = opts->flags.leading_zeros ? '0' : ' ';
    for (size_t i = len; i < opts->min_width; ++i) {
        vga_putchar(leading);
    }

    vga_write(buf, len);
    return len < opts->min_width ? opts->min_width : len;
}

static size_t format_uint(const struct format_options* opts, uintmax_t value) {
    _Static_assert(sizeof(uintmax_t) == sizeof(uint64_t));
    char buf[UINTMAX_DIGITS] = {0};
    size_t digit = UINTMAX_DIGITS;

    do {
        uint64_t rem;
        value = udivmod64(value, 10, &rem);
        buf[--digit] = '0' + (char) rem;
    } while (value > 0);

    return write_uint_buf(opts, &buf[digit], UINTMAX_DIGITS - digit);
}

static size_t format_hex(const struct format_options* opts, uintmax_t value, bool upper) {
    char buf[UINTMAX_NIBBLES] = {0};
    char letter_base = (upper ? 'A' : 'a') - 10;

    size_t digit = UINTMAX_NIBBLES;
    do {
        char nibble = (char)(value & 0xF);
        buf[--digit] = nibble + (nibble < 10 ? '0' : letter_base);
        value >>= 4;
    } while (value > 0);

    return write_uint_buf(opts, &buf[digit], UINTMAX_NIBBLES - digit);
}

static bool parse_format_options(const char* format, struct format_options* opts, const char** format_end) {
    opts->flags.leading_zeros = false;

    switch (*format) {
        case '0':
            opts->flags.leading_zeros = true;
            ++format;
            break;
        case 0:
            return false;
    }

    opts->min_width = strtozu(format, &format);
    if (format == NULL) {
        return false;
    }

    opts->length_modifier = LENGTH_INT;
    switch (*format) {
        case 'h':
            if (format[1] == 'h') {
                ++format;
                opts->length_modifier = LENGTH_CHAR;
            } else {
                opts->length_modifier = LENGTH_SHORT;
            }
            ++format;
            break;
        case 'l':
            if (format[1] == 'l') {
                ++format;
                opts->length_modifier = LENGTH_LONG_LONG;
            } else {
                opts->length_modifier = LENGTH_LONG;
            }
            ++format;
            break;
        case 'j':
            opts->length_modifier = LENGTH_UMAX;
            ++format;
            break;
        case 'z':
            opts->length_modifier = LENGTH_SIZE;
            ++format;
            break;
        case 0:
            return false;
    }

    if (!*format) {
        return false;
    }

    opts->conversion_specifier = *format++; // Parsed later
    *format_end = format;
    return true;
}

static intmax_t read_int_arg(va_list* args, enum format_length type) {
    switch (type) {
        case LENGTH_CHAR:
        case LENGTH_SHORT:
        case LENGTH_INT:
            return va_arg(*args, int);
        case LENGTH_LONG:
            return va_arg(*args, long);
        case LENGTH_LONG_LONG:
            return va_arg(*args, long long);
        case LENGTH_SIZE:
            return va_arg(*args, signed long); // signed size_t
        case LENGTH_UMAX:
            return va_arg(*args, intmax_t);
    }

    __builtin_unreachable();
}

static uintmax_t read_uint_arg(va_list* args, enum format_length type) {
    switch (type) {
        case LENGTH_CHAR:
        case LENGTH_SHORT:
        case LENGTH_INT:
            return va_arg(*args, unsigned int);
        case LENGTH_LONG:
            return va_arg(*args, unsigned long);
        case LENGTH_LONG_LONG:
            return va_arg(*args, unsigned long long);
        case LENGTH_SIZE:
            return va_arg(*args, size_t);
        case LENGTH_UMAX:
            return va_arg(*args, uintmax_t);
    }

    __builtin_unreachable();
}

size_t printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    size_t printed = 0;

    while (*format) {
        const char c = *format++;
        if (c != '%') {
            vga_putchar(c);
            ++printed;
            continue;
        }

        struct format_options opts;
        if (!parse_format_options(format, &opts, &format)) {
            return SIZE_MAX;
        }

        switch (opts.conversion_specifier) {
            case '%':
                vga_putchar('%');
                ++printed;
                break;
            case 'i':
            case 'd': {
                intmax_t value = read_int_arg(&args, opts.length_modifier);
                if (value < 0) {
                    // Manually perform the signed two's complement abs to
                    // avoid overflow problems
                    vga_putchar('-');
                    printed += 1 + format_uint(&opts, ~((uintmax_t) value) + 1);
                } else {
                    printed += format_uint(&opts, value);
                }
                break;
            }
            case 'u':
                printed += format_uint(&opts, read_uint_arg(&args, opts.length_modifier));
                break;
            case 'x':
            case 'X':
                printed += format_hex(&opts, read_uint_arg(&args, opts.length_modifier), opts.conversion_specifier == 'X');
                break;
            case 'p': {
                void* ptr = va_arg(args, void*);
                vga_print("0x");
                opts.min_width = sizeof(intptr_t) * 2;
                opts.flags.leading_zeros = true;
                printed += 2 + format_hex(&opts, (uintptr_t) ptr, true);
                break;
            }
            case 's': {
                const char* str = va_arg(args, const char*);
                size_t len = strlen(str);
                for (size_t i = len; i < opts.min_width; ++i) {
                    vga_putchar(' ');
                }

                vga_print(str);
                printed += len < opts.min_width ? opts.min_width : len;
                break;
            }
            case 'c': {
                uintmax_t value = read_uint_arg(&args, opts.length_modifier);
                vga_putchar(value <= 255 ? (char) value : '?');
                ++printed;
                break;
            }
            default:
                return SIZE_MAX;
        }
    }

    va_end(args);
    return printed;
}
