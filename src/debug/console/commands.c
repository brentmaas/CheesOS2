#include "debug/console/commands.h"
#include "debug/console/console.h"

#include <string.h>

int command_echo(const char* line){
    if(strlen(line) > 5){
        const char* to_print = line + 5;
        console_print(to_print); //Envvars komen later wel(TM)
        console_print("\n");
    }
    return 0;
}

int command_exit(const char* line){
    console_print("Exiting CheeSH...\n");
    return -1;
}

int command_oef(const char* line){
    console_print("Jij hebt aids\n");
    return 0;
}