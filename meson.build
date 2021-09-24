project(
    'CheesOS2',
    'c',
    default_options: [
        'buildtype=debugoptimized',
        'c_std=gnu11',
    ]
)

if not meson.is_cross_build()
    error('CheesOS2 should be cross-compiled: meson CheesOS2 build --croiss-file CheesOS2/i486-unknown-elf-cross.ini')
endif

add_project_arguments(
    '-Wno-unused-const-variable',
    '-Wno-unused-parameter',
    '-Wno-unused-function',
    language: 'c',
)

sources = files(
    'src/core/entry.c',
    'src/core/multiboot.c',
    'src/core/panic.c',
    'src/debug/console/console.c',
    'src/debug/assert.c',
    'src/debug/log.c',
    'src/debug/memdump.c',
    'src/driver/serial/serial.c',
    'src/driver/vga/io.c',
    'src/driver/vga/palette.c',
    'src/driver/vga/text.c',
    'src/driver/vga/util.c',
    'src/driver/vga/videomode.c',
    'src/interrupt/exceptions.c',
    'src/interrupt/idt.c',
    'src/interrupt/pic.c',
    'src/libc/math/udivmod64.c',
    'src/libc/string/mem.c',
    'src/libc/string/string.c',
    'src/libc/stdlib/strtoz.c',
    'src/pci/pci.c',
    'src/ps2/controller.c',
    'src/ps2/device.c',
    'src/ps2/keyboard.c',
    'src/shell/shell.c',
    'src/memory/address_range.c',
    'src/memory/gdt.c',
    'src/memory/pmm.c',
    'src/memory/vmm.c',
    'src/utility/containers/rbtree.c',
    'src/utility/containers/ringbuffer.c',
    'src/utility/cprintf.c',
)

asm_sources = files(
    'src/core/bootstrap.asm',
    'src/interrupt/interrupt.asm',
    'src/libc/crti.asm',
    'src/libc/crtn.asm',
    'src/memory/gdt_load.asm',
    'src/memory/page_table.asm',
)

font_sources = files(
    'res/fonts/cheesos.png',
)

assembler = generator(
    find_program('nasm'),
    output: '@PLAINNAME@.o',
    depfile: '@PLAINNAME@.o.d',
    arguments: [
        '-f', meson.get_cross_property('elfclass'),
        '-MQ', '@OUTPUT@',
        '-MF', '@DEPFILE@',
        '@EXTRA_ARGS@',
        '@INPUT@',
        '-o', '@OUTPUT@'
    ],
)

fonts = custom_target(
    'fonts.[hc]',
    input: font_sources,
    output: ['fonts.h', 'fonts.c'],
    command: [
        find_program('python3'),
        files('tools/createcharmap.py'),
        '-H', '@OUTPUT0@',
        '-C', '@OUTPUT1@',
        '-I', 'fonts.h',
        font_sources,
    ]
)

linker_script = meson.source_root() / 'linker/kernel.ld'

asm_objects = []
foreach src : asm_sources
    asm_objects += assembler.process(src)
endforeach

kernel = executable(
    'kernel',
    [sources, fonts, asm_objects],
    include_directories: include_directories('include', 'include/libc'),
    c_args: ['-ffreestanding', '-nostdlib'],
    link_args: ['-ffreestanding', '-nostdlib', '-T', linker_script],
    link_depends: linker_script,
)

qemu = find_program('qemu')
qemu_common_flags = [
    '-no-reboot',
    '-cpu', meson.get_cross_property('qemu_cpu'),
    '-serial', 'stdio',
    '-m', '12M',
    '-kernel', kernel,
]

run_target(
    'run',
    command: [qemu, qemu_common_flags],
)

run_target(
    'run-debug',
    command: [qemu, qemu_common_flags, '-gdb tcp::1234', '-S', '-d', 'int'],
)
