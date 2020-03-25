#ifndef _CHEESOS2_CORE_IO_H
#define _CHEESOS2_CORE_IO_H

#include <stdint.h>

#define IO_MAKE_ROUTINE(suffix, io_type) \
    static inline io_type io_in##suffix(uint16_t port) { \
        io_type value; \
        asm volatile("in %[port], %[value]" : [value] "=a" (value) : [port] "Nd" (port)); \
        return value; \
    } \
    \
    static inline void io_out##suffix(uint16_t port, io_type value) { \
        asm volatile("out %[value], %[port]" : : [value] "a" (value), [port] "Nd" (port)); \
    }

IO_MAKE_ROUTINE(8, uint8_t) // io_in8(port), io_out8(port, value)
IO_MAKE_ROUTINE(16, uint16_t) // io_in16(port), io_out16(port, value)
IO_MAKE_ROUTINE(32, uint32_t) // io_in32(port), io_out32(port, value)

#undef IO_MAKE_ROUTINE
#undef IO_INTEL_ASM

#endif
