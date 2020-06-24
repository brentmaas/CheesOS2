#include "driver/console/console.h"
#include "vga/vga.h"
#include "stdio.h"
#include "string.h"

int is_command(const char* input, const char* command){
	size_t len = strlen(command);
	return strncmp(input, command, strlen(command)) == 0 && (input[len] == ' ' || input[len] == '\0');
}

void console(void){
	vga_print("\nCheeSH v0.0\nSucces met een console gebruiken zonder toetsinvoer lmao");
	while(1){
		vga_print("\n> ");
		
		//Totdat er invoerinterrupts zijn
		const char* input = "oef";
		vga_print(input);
		vga_print("\n");
		
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