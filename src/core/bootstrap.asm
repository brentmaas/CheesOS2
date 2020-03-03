[BITS 32]

EXTERN kernel_main

EXTERN _init
EXTERN _fini

GLOBAL _start

SECTION .bss
ALIGN 16
__kernel_stack_bottom:
RESB 16*1024
__kernel_stack_top:

SECTION .text

_start:
    mov esp, __kernel_stack_top

    call _init

    push ebx
    call kernel_main
    
    call _fini
    
    cli
    hlt

GLOBAL test_exception
test_exception:
    xor eax, eax
    mov edx, 1
    mov ecx, 2
    mov ebx, 3
    mov esi, 4
    mov edi, 5
    test eax, eax
    ud2