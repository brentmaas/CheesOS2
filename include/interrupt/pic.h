#ifndef _CHEESOS2_INTERRUPT_PIC_H
#define _CHEESOS2_INTERRUPT_PIC_H

#include <stdint.h>

#define PIC_MASTER (0x00)
#define PIC_SLAVE (0x01)

void pic_remap(uint8_t master, uint8_t slave);
void pic_set_mask(uint16_t mask);
void pic_end_interrupt(uint8_t interrupt);

#endif
