#include "debug/console/shell.h"
#include "debug/console/console.h"

#include "ps2/scancode.h"

#include <string.h>
#include <stdbool.h>

static int is_command(char* input, const char* command) {
    size_t len = strlen(command);
    return strncmp(input, command, strlen(command)) == 0 && (input[len] == ' ' || input[len] == '\0');
}

void console_shell_loop(void) {
    console_print("CheeSH v0.1\nKan niet wachten tot m'n shitcode gepurged wordt\n");
    while (true) {
        console_print("> ");

        char input[100] = {'\0'};
        size_t cursor = 0;
        char last = '\0';
        while (last != '\n') {
            last = ps2_read_char_on_release();
            if (cursor < 100 && last) {
                if (last != '\n') input[cursor] = last;
                ++cursor;
                console_putchar(last);
            }
        }

        if (is_command(input, "exit")) {
            console_print("Exiting CheeSH...\n");
            return;
        } else if (is_command(input, "echo")) {
            if (strlen(input) > 5) {
                const char* to_print = input + 5;
                console_print(to_print); // Envvars komt later wel(TM)
                console_print("\n");
            }
        } else if (is_command(input, "oef")) {
            console_print("Jij hebt aids\n");
        } else {
            console_printf("Unknown command: %s\n", input);
        }
    }
}
