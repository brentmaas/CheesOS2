ENTRY(_start)

SECTIONS
{
    kernel_virtual_start = 0xC0000000;

    . = 1M;
    kernel_start = .;

    .bootstrap.text : ALIGN(4K)
    {
        *(.multiboot)
        *(.bootstrap.text)
    }

    . += kernel_virtual_start;

    .text : AT(ADDR(.text) - kernel_virtual_start) ALIGN(4K)
    {
        *(.text)
        *(.init)
        *(.fini)
    }

    .rodata : AT(ADDR(.rodata) - kernel_virtual_start) ALIGN(4K)
    {
        *(.rodata)
    }

    .data : AT(ADDR(.data) - kernel_virtual_start) ALIGN(4K)
    {
        *(.data)
    }

    .bss : AT(ADDR(.bss) - kernel_virtual_start) ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
    }

    kernel_end = . - kernel_virtual_start;
}

ASSERT(kernel_end < 1024 * 4096, "Kernel does not fit into 4 MiB");
