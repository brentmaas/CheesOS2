#include "driver/console/console.h"
#include "vga/vga.h"
#include "ps2/scancode.h"

#include <string.h>

int is_command(char* input, const char* command){
    size_t len = strlen(command);
    return strncmp(input, command, strlen(command)) == 0 && (input[len] == ' ' || input[len] == '\0');
}

void console(void){
    vga_print("\nCheeSH v0.1\nKan niet wachten tot m'n shitcode gepurged wordt");
    while(1){
        vga_print("\n> ");

        char input[100] = {'\0'};
        size_t cursor = 0;
        char last = '\0';
        while(last != '\n'){
            last = ps2_read_char_on_release();
            if(cursor < 100 && last){
                if(last != '\n') input[cursor] = last;
                ++cursor;
                vga_putchar(last);
            }
        }

        if(is_command(input, "exit")){
            vga_print("Exiting CheeSH...");
            return;
        }else if(is_command(input, "echo")){
            if(strlen(input) > 5){
                const char* to_print = input + 5;
                vga_print(to_print); //Envvars komt later wel(TM)
            }else{
                vga_print("\n");
            }
        }else if(is_command(input, "oef")){
            vga_print("Jij hebt aids\n");
        }else{
            vga_print("Unknown command: ");
            vga_print(input);
            vga_print("\n");
        }
    }
}