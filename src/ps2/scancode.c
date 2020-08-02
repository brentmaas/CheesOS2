#include "ps2/scancode.h"
#include "ps2/controller.h"

//Zie https://wiki.osdev.org/PS/2_Keyboard
//Deze moet nog afgemaakt worden
typedef enum {
	PS2_KEY_F9 = 0x1,
	PS2_KEY_F5 = 0x3,
	PS2_KEY_F3 = 0x4,
	PS2_KEY_F1 = 0x5,
	PS2_KEY_F2 = 0x6,
	PS2_KEY_F12 = 0x7
} ps2_scancode2;

const char ps2_scancode_set2[128] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '\t', '`', 0,
									 0, 0, 0, 0, 0, 'q', '1', 0, 0, 0, 'z', 's', 'a', 'w', '2', 0,
									 0, 'c', 'x', 'd', 'e', '4', '3', 0, 0, ' ', 'v', 'f', 't', 'r', '5', 0,
									 0, 'n', 'b', 'h', 'g', 'y', '6', 0, 0, 0, 'm', 'j', 'u', '7', '8', 0,
									 0, ',', 'k', 'i', 'o', '0', '9', 0, 0, '.', '/', 'l', ';', 'p', '-', 0,
									 0, 0, '\'', 0, '[', '=', 0, 0, 0, 0, '\n', ']', 0, 0, 0, 0,
									 0, 0, 0, 0, 0, 0, 0, 0, 0, '1', 0, '4', '7', 0, 0, 0,
									 '0', '.', '2', '5', '6', '8', 0, 0, 0, '+', '3', '-', '*', '9', 0, 0};

//TODO const char ps2_scancode_set2_uppercase

const uint8_t PS2_RELEASE = 0xf0;

char ps2_read_char_on_press(){
	char out = 0;
	while(!out){
		ps2_controller_wait_input();
		uint8_t scancode = ps2_controller_read();
		if(scancode != PS2_RELEASE) out = ps2_scancode_set2[scancode];
		else{ //Negeer release
			ps2_controller_wait_input();
			ps2_controller_read();
		}
	}
	return out;
}

char ps2_read_char_on_release(){
	uint8_t scancode = 0;
	while(scancode != PS2_RELEASE){
		ps2_controller_wait_input();
		scancode = ps2_controller_read();
	}
	ps2_controller_wait_input();
	scancode = ps2_controller_read();
	return ps2_scancode_set2[scancode];
}