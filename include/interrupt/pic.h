#ifndef _CHEESOS2_INTERRUPT_PIC_H
#define _CHEESOS2_INTERRUPT_PIC_H

#include <stdint.h>

void pic_remap(uint8_t master, uint8_t slave);
void pic_set_mask(uint16_t mask);

#endif
