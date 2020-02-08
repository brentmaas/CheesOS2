#ifndef _CHEESOS2_CORE_IO_H
#define _CHEESOS2_CORE_IO_H

#include <stdint.h>

#define IO_INTEL_ASM(asm_code) ".intel_syntax noprefix\n" asm_code "\n.att_syntax"

#define IO_MAKE_ROUTINE(suffix, io_type) \
    static inline io_type io_in##suffix(uint16_t port) { \
        io_type value; \
        asm volatile(IO_INTEL_ASM("in %[value], %[port]") : [value] "=a" (value) : [port] "Nd" (port)); \
        return value; \
    } \
    \
    static inline void io_out##suffix(uint16_t port, io_type value) { \
        asm volatile(IO_INTEL_ASM("out %[port], %[value]") : : [value] "a" (value), [port] "Nd" (port)); \
    }

IO_MAKE_ROUTINE(8, uint8_t)
IO_MAKE_ROUTINE(16, uint16_t)
IO_MAKE_ROUTINE(32, uint32_t)

#undef IO_MAKE_ROUTINE
#undef IO_INTEL_ASM

#endif
