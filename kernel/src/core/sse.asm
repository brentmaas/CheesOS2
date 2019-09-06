[BITS 64]

; Exports
global sse_init_native
global sse_init_avx_native

section .text

sse_init_native:
    ; Fetch the old value of cr0
    mov rax, cr0

    ; Clear CR0.EM
    and rax, ~(1 << 2)

    ; Set CR0.MP
    or rax, (1 << 1)

    ; Store the result
    mov cr0, rax

    ; Fetch the old value of cr4
    mov rax, cr4

    ; Set CR4.OSFXSR and CR4.OSXMMEXCPT
    or rax, (3 << 9)

    ; Store the new value of cr4
    mov cr4, rax
    ret

sse_init_avx_native:
    ret