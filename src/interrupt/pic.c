#include "interrupt/pic.h"
#include "core/io.h"

#define PIC_MASTER_COMMAND_PORT (0x20)
#define PIC_MASTER_DATA_PORT (0x21)
#define PIC_SLAVE_COMMAND_PORT (0xA0)
#define PIC_SLAVE_DATA_PORT (0xA1)

#define PIC_CONTROL1_TOGGLE_4 (0x01)
#define PIC_CONTROL1_CASCADE (0x2)
#define PIC_CONTROL1_CALL_INTERVAL4 (0x04)
#define PIC_CONTROL1_LEVELED (0x08)
#define PIC_CONTROL1_INIT (0x10)

#define PIC_CONTROL4_8086 (0x01)
#define PIC_CONTROL4_AUTO_EOI (0x02)
#define PIC_CONTROL4_SLAVE_BUFFERED (0x08)
#define PIC_CONTROL4_MASTER_BUFFERED (0x0C)
#define PIC_CONTROL4_TOGGLE_SPECIAL_FULLY_NESTED (0x10)

#define PIC_MASTER_SLAVE_IRQ (0x04)
#define PIC_SLAVE_CASCADE_IRQ (0x02)

void pic_remap(uint8_t master, uint8_t slave) {
    uint8_t master_mask = io_in8(PIC_MASTER_DATA_PORT);
    uint8_t slave_mask = io_in8(PIC_SLAVE_DATA_PORT);

    io_out8(PIC_MASTER_COMMAND_PORT, PIC_CONTROL1_INIT | PIC_CONTROL1_TOGGLE_4);
    io_out8(PIC_SLAVE_COMMAND_PORT, PIC_CONTROL1_INIT | PIC_CONTROL1_TOGGLE_4);
    io_out8(PIC_MASTER_DATA_PORT, master);
    io_out8(PIC_SLAVE_DATA_PORT, slave);
    io_out8(PIC_MASTER_DATA_PORT, PIC_MASTER_SLAVE_IRQ);
    io_out8(PIC_SLAVE_DATA_PORT, PIC_SLAVE_CASCADE_IRQ);

    io_out8(PIC_MASTER_DATA_PORT, PIC_CONTROL4_8086);
    io_out8(PIC_SLAVE_DATA_PORT, PIC_CONTROL4_8086);

    io_out8(PIC_MASTER_DATA_PORT, master_mask);
    io_out8(PIC_SLAVE_DATA_PORT, slave_mask);
}

void pic_set_mask(uint16_t mask) {
    uint8_t master_mask = mask & 0xFF;
    uint8_t slave_mask = mask & 0xFF00 >> 8;

    io_out8(PIC_MASTER_DATA_PORT, master_mask);
    io_out8(PIC_SLAVE_DATA_PORT, slave_mask);
}