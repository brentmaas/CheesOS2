#include "debug/memdump.h"
#include "vga/vga.h"

static char hex_digit(uint8_t v) {
    if(v >= 10)
        return 'A' + v - 10;
    else
        return '0' + v;
}

void debug_memdump(void* addr, size_t size) {
    uint8_t* data = (uint8_t*)addr;
    for(size_t i = 0; i < size; ++i) {
        if(i % 16 == 0 && i > 0) {
            vga_print("\n");
        }

        char value[4];
        value[0] = hex_digit((data[i] & 0xF0u) >> 4u);
        value[1] = hex_digit((data[i] & 0xFu));
        value[2] = ' ';
        value[3] = '\0';
        vga_print(value);
    }
}