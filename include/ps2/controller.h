#ifndef _CHEESOS2_PS2_CONTROLLER_H
#define _CHEESOS2_PS2_CONTROLLER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

int ps2_controller_init(void);
void ps2_controller_wait_input(void);
uint8_t ps2_controller_read(void);

#endif
