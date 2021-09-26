#include "shell/shell.h"

#include "ps2/keyboard.h"

#include "string.h"

#include "utility/containers/ringbuffer.h"

#include "debug/console/console.h"

volatile static bool loop = true;

void shell_do_command(uint8_t* command, size_t length){
    //log_debug("Full line: '%s'", command);
    
    size_t command_length = length;
    for(size_t i = 0;i < length;++i){
        if(command[i] == ' '){
            command_length = i;
            break;
        }
    }
    
    int argc = 1;
    bool whitespace = true;
    for(size_t i = command_length;i < length;++i){
        if((command[i] != ' ' && command[i] != '\0') && whitespace){
            whitespace = false;
            ++argc;
        }else if(command[i] == ' ' || command[i] == '\0'){
            whitespace = true;
            command[i] = '\0';
        }
    }
    
    char* argv[argc];
    argv[0] = (char*) command;
    size_t j = 1;
    for(size_t i = command_length;i < length;++i){
        if(command[i] != '\0' && command[i-1] == '\0'){
            argv[j] = (char*) &command[i];
            ++j;
        }
    }
    
    /*log_debug("Command: '%s', length=%u, argc=%i", command, command_length, argc);
    for(int i = 0;i < argc;++i){
        log_debug("    argv[%i]='%s'", i, argv[i]);
    }*/
    
    //Builtin commands
    if(!strncmp(argv[0], "exit", command_length)){
        loop = false;
    }else if(!strncmp(argv[0], "echo", command_length)){
        if(argc > 1){
            for(size_t i = command_length;i < length;++i){
                if(command[i] == '\0') command[i] = ' ';
            }
            console_printf("%s", argv[1]);
        }
        console_putchar('\n');
    }else if(!strncmp(argv[0], "help", command_length)){
        console_print("'no'\n");
    }else{
        //TODO: run executables
        console_printf("Unknown command '%s'\n", argv[0]);
    }
}

void shell_print_header(void){
    console_print("Demo user> ");
}

void shell_loop(void){
    ringbuffer rbuffer;
    ringbuffer_init(&rbuffer);
    
    console_print("\nCheeSH v0.2\nPage Fault-editie\n\n");
    shell_print_header();
    
    loop = true;
    while(loop){
        console_print_cursor();
        uint8_t next;
        bool is_release;
        ps2_keyboard_get_next_char(&next, &is_release);
        if(!is_release){
            if(next == '\n'){
                console_clear_cursor();
                console_putchar('\n');
                size_t length = ringbuffer_length(&rbuffer);
                uint8_t line[length+1];
                line[length] = 0;
                ringbuffer_read(&rbuffer, line, length);
                shell_do_command(line, length);
                if(loop) shell_print_header();
            }else if(next == 8 && ringbuffer_length(&rbuffer) > 0){
                ringbuffer_remove(&rbuffer, 1);
                console_clear_cursor();
                console_backspace();
            }else if(next != 8){
                console_putchar(next);
                ringbuffer_put(&rbuffer, next);
            }
        }
    }
}