section .multiboot
align 4
    ; Multiboot header
    dd 0x1BADB002              ; Magic number
    dd 0x00                    ; Flags (0 = no special features)
    dd -(0x1BADB002 + 0x00)    ; Checksum

section .bss
align 16
stack_bottom:
    resb 16384 ; 16 KB
stack_top:

section .text
global _start
extern kernel_main

_start:
    ; Set up stack
    mov esp, stack_top

    ; Push Multiboot information
    push ebx    ; Multiboot info structure
    push eax    ; Multiboot magic number

    ; Call kernel main
    call kernel_main

    ; If kernel_main returns, halt
    cli
.hang:
    hlt
    jmp .hang 