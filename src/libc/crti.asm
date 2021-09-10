[BITS 32]

GLOBAL _init
GLOBAL _fini

SECTION .init.pre
_init:
    push ebp
    mov ebp, esp

SECTION .fini.pre
_fini:
    push ebp
    mov ebp, esp