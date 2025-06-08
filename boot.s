.section .multiboot
.align 4
multiboot_header:
    .long 0x1BADB002            # magic number
    .long 0x0                   # flags
    .long -(0x1BADB002 + 0x0)   # checksum

.section .text
.global _start
_start:
    mov $stack_top, %esp
    call kernel_main

hang:
    jmp hang

.section .bss
    .align 16
stack_bottom:
    .skip 16384
stack_top:
