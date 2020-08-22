#ifndef _CHEESOS2_PS2_CONTROLLER_H
#define _CHEESOS2_PS2_CONTROLLER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

int ps2_controller_init(void);
void ps2_controller_wait_output(void);
bool ps2_controller_has_input(void);

#endif
