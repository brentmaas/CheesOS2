#ifndef _CHEESOS2_CORE_IO_H
#define _CHEESOS2_CORE_IO_H

#include <stdint.h>

#define IO_MAKE_ROUTINE(io_name, io_type) \
    static inline io_type io_in_##io_name(uint16_t port) { \
        io_type value; \
        asm volatile("in %[value], %[port]" : [value] "=a" (value) : [port] "Nd" (port)); \
        return value; \
    } \
    \
    static inline void io_out_##io_name(uint16_t port, io_type value) { \
        asm volatile("out %[port], %[value]" : : [value] "a" (value), [port] "Nd" (port)); \
    }

IO_MAKE_ROUTINE(u8, uint8_t)
IO_MAKE_ROUTINE(u16, uint16_t)
IO_MAKE_ROUTINE(u32, uint32_t)

#undef IO_MAKE_ROUTINE

#endif
