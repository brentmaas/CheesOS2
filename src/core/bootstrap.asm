EXTERN kernel_main

EXTERN _init
EXTERN _fini

GLOBAL _start

SECTION .bss
ALIGN 16
__kernel_stack_bottom:
RESB 16*1026
__kernel_stack_top:

SECTION .text

_start:
    mov esp, __kernel_stack_top

    call _init
    call kernel_main
    call _fini
    
    cli
    hlt
