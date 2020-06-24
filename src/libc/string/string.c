#include <string.h>
#include <stdio.h>

size_t strlen(const char* str) {
    size_t count = 0;
    while(*str++) {
        ++count;
    }
    return count;
}

int strcmp(const char* lhs, const char* rhs){
	size_t i = 0;
	while(lhs[i] && lhs[i] == rhs[i]){
		++i;
	}
	return (int) lhs[i] - (int) rhs[i];
}

int strncmp(const char* lhs, const char* rhs, size_t count){
	size_t i = 0;
	while(lhs[i] > 0 && lhs[i] == rhs[i] && i < count - 1){
		++i;
	}
	return (int) lhs[i] - (int) rhs[i];
}
