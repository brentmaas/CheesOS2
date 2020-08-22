#ifndef _CHEESOS2_DEBUG_CONSOLE_SHELL_H
#define _CHEESOS2_DEBUG_CONSOLE_SHELL_H

enum command_status {
    COMMAND_STATUS_OK = 0,
    COMMAND_STATUS_SHUTDOWN = -1
};

void console_shell_loop(void);

#endif
