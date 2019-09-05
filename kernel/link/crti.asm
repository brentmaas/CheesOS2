; gcc global constructor/destructor initialization stub

[BITS 64]

global _init
global _fini

section .init
_init:
    push rbp
    mov rbp, rsp

    ; Gcc will insert global constructor calls here

section .fini
_fini:
    push rbp
    mov rbp, rsp

    ; Gcc will insert global destructor calls here