#include "debug/console/shell.h"
#include "debug/console/console.h"

#include "ps2/scancode.h"

#include <string.h>

#include "debug/console/commands.h"
int command_help(const char* line);

struct command {
    const char* command_name;
    int (*command_func)(const char* line);
};

struct command commands[] = {{"echo", command_echo},
                             {"exit", command_exit},
                             {"help", command_help},
                             {"oef", command_oef}};

static int is_command(char* input, const char* command) {
    size_t len = strlen(command);
    return strncmp(input, command, strlen(command)) == 0 && (input[len] == ' ' || input[len] == '\0');
}

void console_shell_loop(void) {
    console_print("CheeSH v0.1\nKan niet wachten tot m'n shitcode gepurged wordt\n");
    while (1) {
        console_print("> ");

        char input[100] = {'\0'};
        size_t cursor = 0;
        int last = 0;
        char last_char = '\0';
        while (last_char != '\n') {
            last = ps2_read_on_release();
            last_char = ps2_get_char(last);
            if (cursor < 100 && last_char) {
                if (last_char != '\n') input[cursor] = last_char;
                ++cursor;
                console_putchar(last_char);
            }else if(last == PS2_KEY_BACKSPACE && cursor > 0){
                --cursor;
                input[cursor] = '\0';
                console_erasechar();
            }
        }
        
        int ret = 0, found = 0;
        for(size_t i = 0;i < sizeof(commands)/sizeof(struct command) && !found;++i){
            if(is_command(input, commands[i].command_name)){
                found = 1;
                ret = (*commands[i].command_func)(input);
            }
        }
        if(!found){
            console_printf("Unknown command: %s\nTry 'help' for a list of commands\n", input);
        }
        if(ret == COMMAND_STATUS_SHUTDOWN) return;
    }
}

int command_help(const char* line){
    for(size_t i = 0;i < sizeof(commands)/sizeof(struct command);++i){
        console_print(commands[i].command_name);
        console_print("\n");
    }
    return 0;
}