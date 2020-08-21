#include <string.h>

size_t strlen(const char* str) {
    size_t count = 0;
    while(*str++) {
        ++count;
    }
    return count;
}

int strcmp(const char* lhs, const char* rhs){
	while(*lhs && (*lhs == *rhs)) {
        ++lhs;
        ++rhs;
    }
    return *lhs - *rhs;
}

int strncmp(const char* lhs, const char* rhs, size_t count){
    while(count && *lhs && (*lhs == *rhs)) {
        --count;
        ++lhs;
        ++rhs;
    }
    if(count == 0)
        return 0;
    return *lhs - *rhs;
}
