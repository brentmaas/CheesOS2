#include "shell/shell.h"

#include "ps2/keyboard.h"

#include "string.h"

#include "utility/containers/ringbuffer.h"

#include "debug/console/console.h"

volatile static bool loop = true;

void shell_do_command(uint8_t* command, size_t length){
    //log_debug("Full line: '%s'", command);
    
    int argc = 0;
    int command_length = length;
    for(size_t i = 0;i < length - 1;++i) if(command[i] == ' '){
        command_length = i + 1;
        break;
    }
    bool whitespace = true;
    for(size_t i = command_length - 1;i < length;++i){
        if((command[i] == ' ' || command[i] == '\0') && !whitespace){
            ++argc;
            command[i] = '\0';
            whitespace = true;
        }else if(command[i] != ' ' && command[i] != '\0'){
            whitespace = false;
        }else{
            command[i] = '\0';
        }
    }
    char* argv[argc];
    argv[0] = (char*) &command[command_length];
    int j = 1;
    for(size_t i = command_length + 1;i < length - 1;++i) if(command[i] == '\0'){
        argv[j] = (char*) &command[i+1];
        ++j;
    }
    
    //log_debug("Command: '%s', length=%i, argc=%i", command, command_length, argc);
    for(int i = 0;i < argc;++i){
        //log_debug("    argv[%i]='%s'", i, argv[i]);
    }
    
    //Builtin commands
    if(!strncmp((char*) command, "exit", command_length)){
        loop = false;
    }else if(!strncmp((char*) command, "echo", command_length)){
        if(argc > 0){
            for(int i = 0;i < argc;++i){
                if(i > 0) console_putchar(' ');
                console_printf("%s", argv[i]);
            }
        }
        console_putchar('\n');
    }else if(!strncmp((char*) command, "help", command_length)){
        console_print("'no'\n");
    }else{
        //TODO: run executables
        console_printf("Unknown command '%s'\n", command);
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
                shell_do_command(line, length + 1);
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