EXTERN kernel_main

GLOBAL _start

SECTION .bss
ALIGN 16
__kernel_stack_bottom:
RESB 16*1026
__kernel_stack_top:

SECTION .text

_start:
    mov esp, __kernel_stack_top
    
    call kernel_main
    
    cli
    hlt
