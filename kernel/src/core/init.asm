[BITS 64]

; Imports
extern kernel_main

; Exports
global _start

; Spectial section for the intial kernel stack
section .bss
    ; Reserve 16kb
    resb 16*1024
kernel_stack_end:

; Start of the kernel
section .text

_start:
    ; rdi should point to the hardware information structure

    ; Initialize the kernel stack
    lea rsp, [kernel_stack_end]
    xor rbp, rbp

    ; Should not return
    call kernel_main

    ; Halt
    cli
    hlt