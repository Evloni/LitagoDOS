section .multiboot
align 4
    ; Multiboot header
    dd 0x1BADB002              ; Magic number
    dd 0x04                    ; Flags (0x04 = VBE mode)
    dd -(0x1BADB002 + 0x04)    ; Checksum

    ; VBE information
    dd 0                       ; Header address
    dd 0                       ; Load address
    dd 0                       ; Load end address
    dd 0                       ; BSS end address
    dd 0                       ; Entry address
    dd 0                       ; Mode type
    dd 1024                    ; Width
    dd 768                     ; Height
    dd 32                      ; Depth

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