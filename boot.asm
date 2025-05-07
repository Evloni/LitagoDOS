[BITS 32]
[ORG 0x7C00]

; Multiboot header
section .multiboot
align 4
    ; Magic number
    dd 0x1BADB002
    ; Flags
    dd 0x00
    ; Checksum
    dd -(0x1BADB002 + 0x00)

section .bss
align 16
stack_bottom:
    resb 16384 ; 16 KiB
stack_top:

section .text
global _start
_start:
    mov esp, stack_top
    ; Initialize your kernel here directly
    cli
.halt:
    hlt
    jmp .halt

times 510-($-$$) db 0
dw 0xAA55