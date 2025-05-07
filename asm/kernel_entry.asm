section .multiboot
align 4
    dd 0x1BADB002          ; magic
    dd 0x0                 ; flags
    dd -(0x1BADB002+0x0)   ; checksum

section .text
    global start
    extern kernel_main
start:
    mov esp, stack_top
    call kernel_main
    cli
.hang:
    hlt
    jmp .hang

section .bss
align 16
stack_bottom:
    resb 16384
stack_top: 