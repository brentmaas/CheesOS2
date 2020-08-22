#include "utility/cprintf.h"
#include "debug/assert.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

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

static int write_uint_buf(cprintf_write_cbk cbk, void* ctx, const struct format_options* opts, const char* buf, size_t len) {
    char leading = opts->flags.leading_zeros ? '0' : ' ';
    for (size_t i = len; i < opts->min_width; ++i) {
        int result = cbk(ctx, &leading, 1);
        if (result) {
            return result;
        }
    }

    return cbk(ctx, buf, len);
}

static int format_uint(cprintf_write_cbk cbk, void* ctx, const struct format_options* opts, uintmax_t value) {
    _Static_assert(sizeof(uintmax_t) == sizeof(uint64_t), "format_uint expects uintmax_t == uint64_t");
    char buf[UINTMAX_DIGITS] = {0};
    size_t digit = UINTMAX_DIGITS;

    do {
        uint64_t rem;
        value = udivmod64(value, 10, &rem);
        buf[--digit] = '0' + (char) rem;
    } while (value > 0);

    return write_uint_buf(cbk, ctx, opts, &buf[digit], UINTMAX_DIGITS - digit);
}

static int format_hex(cprintf_write_cbk cbk, void* ctx, const struct format_options* opts, uintmax_t value) {
    char buf[UINTMAX_NIBBLES] = {0};
    bool upper = opts->conversion_specifier == 'X';
    char letter_base = (upper ? 'A' : 'a') - 10;

    size_t digit = UINTMAX_NIBBLES;
    do {
        char nibble = (char)(value & 0xF);
        buf[--digit] = nibble + (nibble < 10 ? '0' : letter_base);
        value >>= 4;
    } while (value > 0);

    return write_uint_buf(cbk, ctx, opts, &buf[digit], UINTMAX_NIBBLES - digit);
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

    unreachable();
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

    unreachable();
}

int vcprintf(cprintf_write_cbk cbk, void* context, const char* format, va_list args) {
    while (*format) {
        const char c = *format++;
        if (c != '%') {
            int result = cbk(context, &c, 1);
            if (result) {
                return result;
            }
            continue;
        }

        struct format_options opts;
        if (!parse_format_options(format, &opts, &format)) {
            return SIZE_MAX;
        }

        switch (opts.conversion_specifier) {
            case '%': {
                int result = cbk(context, "%", 1);
                if (result) {
                    return result;
                }
                break;
            }
            case 'i':
            case 'd': {
                intmax_t value = read_int_arg(&args, opts.length_modifier);
                if (value < 0) {
                    // Manually perform the signed two's complement abs to
                    // avoid overflow problems
                    int result = cbk(context, "-", 1);
                    if (result) {
                        return result;
                    }
                    result = format_uint(cbk, context, &opts, ~((uintmax_t) value) + 1);
                    if (result) {
                        return result;
                    }
                } else {
                    int result = format_uint(cbk, context, &opts, value);
                    if (result) {
                        return result;
                    }
                }
                break;
            }
            case 'u': {
                uintmax_t value = read_uint_arg(&args, opts.length_modifier);
                int result = format_uint(cbk, context, &opts, value);
                if (result) {
                    return result;
                }
                break;
            }
            case 'x':
            case 'X': {
                uintmax_t value = read_uint_arg(&args, opts.length_modifier);
                int result = format_hex(cbk, context, &opts, value);
                if (result) {
                    return result;
                }
                break;
            }
            case 'p': {
                void* ptr = va_arg(args, void*);
                int result = cbk(context, "0x", 2);
                if (result) {
                    return result;
                }
                opts.min_width = sizeof(intptr_t) * 2;
                opts.flags.leading_zeros = true;
                opts.conversion_specifier = 'X';
                result = format_hex(cbk, context, &opts, (uintptr_t) ptr);
                if (result) {
                    return result;
                }
                break;
            }
            case 's': {
                const char* str = va_arg(args, const char*);
                size_t len = strlen(str);
                for (size_t i = len; i < opts.min_width; ++i) {
                    int result = cbk(context, " ", 1);
                    if (result) {
                        return result;
                    }
                }

                int result = cbk(context, str, len);
                if (result) {
                    return result;
                }
                break;
            }
            case 'c': {
                uintmax_t value = read_uint_arg(&args, opts.length_modifier);
                char c = value <= 0xFF ? value : '?';
                int result = cbk(context, &c, 1);
                if (result) {
                    return result;
                }
                break;
            }
            default:
                return FORMAT_ERR_INVALID_SPECIFIER;
        }
    }

    return 0;
}

int cprintf(cprintf_write_cbk cbk, void* context, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int result = vcprintf(cbk, context, format, args);
    va_end(args);
    return result;
}

struct vsnprintf_ctx {
    size_t num_written;
    char* buffer;
    size_t bufsz;
};

int vsnprintf_cbk(void* context, const char* data, size_t size) {
    struct vsnprintf_ctx* ctx = context;

    if (ctx->bufsz > 0 && ctx->num_written < ctx->bufsz - 1) {
        size_t size_left = ctx->bufsz - 1 - ctx->num_written;
        size_t write_size = size_left < size ? size_left : size;
        for (size_t i = 0; i < write_size; ++i) {
            ctx->buffer[ctx->num_written + i] = data[i];
        }
    }

    ctx->num_written += size;
    return 0;
}

int vsnprintf(char* restrict buffer, size_t bufsz, const char* restrict format, va_list args) {
    struct vsnprintf_ctx ctx = {
        .num_written = 0,
        .buffer = buffer,
        .bufsz = bufsz,
    };
    int result = vcprintf(vsnprintf_cbk, &ctx, format, args);
    if (result) {
        return result;
    }

    if (ctx.num_written < bufsz) {
        buffer[ctx.num_written] = 0;
    } else if (bufsz > 0) {
        buffer[bufsz] = 0;
    }

    return ctx.num_written;
}

int snprintf(char* restrict buffer, size_t bufsz, const char* restrict format, ...) {
    va_list args;
    va_start(args, format);
    int result = vsnprintf(buffer, bufsz, format, args);
    va_end(args);
    return result;
}

struct vbprintf_ctx {
    char* buffer;
    size_t size_left;
};

int vbprintf_cbk(void* context, const char* data, size_t size) {
    struct vbprintf_ctx* ctx = context;

    if (ctx->size_left == 0) {
        return 0;
    }

    size_t write_size = ctx->size_left < size ? ctx->size_left : size;
    for (size_t i = 0; i < write_size; ++i) {
        ctx->buffer[i] = data[i];
    }

    ctx->buffer += write_size;
    ctx->size_left -= write_size;
    return 0;
}

int vbprintf(char** restrict buffer, size_t* size_left, const char* restrict format, va_list args) {
    struct vbprintf_ctx ctx = {
        .buffer = *buffer,
        .size_left = *size_left,
    };

    int result = vcprintf(vbprintf_cbk, &ctx, format, args);
    if (ctx.size_left > 0) {
        ctx.buffer[0] = 0;
    }

    *buffer = ctx.buffer;
    *size_left = ctx.size_left;
    return result;
}

int bprintf(char** restrict buffer, size_t* size_left, const char* restrict format, ...) {
    va_list args;
    va_start(args, format);
    int result = vbprintf(buffer, size_left, format, args);
    va_end(args);
    return result;
}