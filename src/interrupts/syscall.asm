global syscall_entry
extern syscall_handler

syscall_entry:
    ; Save registers
    pusha
    push ds
    push es
    push fs
    push gs
    
    ; Set up kernel data segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Call C handler
    push esp
    call syscall_handler
    add esp, 4
    
    ; Restore registers
    pop gs
    pop fs
    pop es
    pop ds
    popa
    
    ; Return to user mode
    iret