#include "driver/console/console.h"
#include "vga/vga.h"
#include "stdio.h"

int should_console_run;

void console_init(void){
	should_console_run = 1;
	vga_print("\nCheeSH v0.0\nSucces met een console gebruiken zonder toetsinvoer lmao");
}

void console_loop(void){
	while(should_console_run){
		vga_print("\n> ");
		should_console_run = 0;
	}
}