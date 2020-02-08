#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include "vga/vga.h"
#include "libc/string.h"

static void format_int(char* buffer, size_t i){
	size_t j = 0;
	if(i < 0){
		i = -i;
		buffer[j] = '-';
		++j;
	}
	char tmp[24];
	size_t k = 0;
	while(i > 0){
		tmp[k] = '0' + (i % 10);
		i /= 10;
		++k;
	}
	for(size_t l = 0;l < k;++l){
		buffer[j+k-l-1] = tmp[l];
	}
}

static void format_hex(char* buffer, size_t x, bool upper){
	if(x < 0){
		x = -x;
	}
	buffer[0] = '0';
	buffer[1] = 'x';
	char tmp[16];
	size_t k = 0;
	while(x > 0){
		size_t f = x % 16;
		if(f < 10){
			tmp[k] = '0' + f;
		}else{
			if(upper){
				tmp[k] = 'A' + f - 10;
			}else{
				tmp[k] = 'a' + f - 10;
			}
		}
		x /= 16;
		++k;
	}
	for(size_t l = 0;l < k;++l){
		buffer[1+k-l] = tmp[l];
	}
}

size_t printf(const char* format, ...){
	va_list args;
	va_start(args, format);
	size_t printed = 0;
	while(*format != '\0'){
		if(*format == '%'){
			++format;
			if(*format == '%'){
				vga_putchar('%');
				++printed;
			}else if(*format == 'i'){
				size_t i = va_arg(args, size_t);
				char f[24] = {'\0'};
				format_int(f, i);
				vga_print(f);
				printed += strlen(f);
			}else if(*format == 's'){
				char* s = va_arg(args, char*);
				vga_print(s);
				printed += strlen(s);
			}else if(*format == 'c'){
				size_t c = va_arg(args, size_t);
				vga_putchar(c);
				++printed;
			}else if(*format == 'x' || *format == 'X'){
				size_t i = va_arg(args, size_t);
				char f[16] = {'\0'};
				format_hex(f, i, *format == 'X');
				vga_print(f);
				printed += strlen(f);
			}else{
				return SIZE_MAX;
			}
		}else{
			vga_putchar(*format);
			++printed;
		}
		++format;
	}
	return printed;
}

size_t snprintf(char* buffer, size_t buffer_size, const char* format, ...){
	return printf("snprintf nog niet gemaakt");
}

size_t sprintf(char* buffer, const char* format, ...){
	return printf("sprintf nog niet gemaakt");
}