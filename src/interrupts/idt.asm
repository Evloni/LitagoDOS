global idt_load
extern idt_ptr

; Load IDT
idt_load:
    lidt [idt_ptr]
    ret

; Interrupt handlers
global irq0
global irq1
; ... add more as needed

; Timer interrupt handler
irq0:
    cli                     ; Disable interrupts
    pusha                   ; Save all registers
    push ds                 ; Save segment registers
    push es
    push fs
    push gs
    
    mov ax, 0x10           ; Load kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Align stack to 16 bytes for C function call
    sub esp, 4
    
    extern timer_handler
    call timer_handler      ; Call C handler
    
    ; Restore stack alignment
    add esp, 4
    
    ; Send EOI to PIC1
    mov al, 0x20
    out 0x20, al
    
    pop gs                  ; Restore segment registers
    pop fs
    pop es
    pop ds
    popa                   ; Restore all registers
    iret                   ; Return from interrupt (sti will be done by iret)

; Keyboard interrupt handler
irq1:
    cli                     ; Disable interrupts
    pusha                   ; Save all registers
    push ds                 ; Save segment registers
    push es
    push fs
    push gs
    
    mov ax, 0x10           ; Load kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Align stack to 16 bytes for C function call
    sub esp, 4
    
    extern keyboard_handler
    call keyboard_handler   ; Call C handler
    
    ; Restore stack alignment
    add esp, 4
    
    ; Send EOI to PIC1
    mov al, 0x20
    out 0x20, al
    
    pop gs                  ; Restore segment registers
    pop fs
    pop es
    pop ds
    popa                   ; Restore all registers
    iret                   ; Return from interrupt (sti will be done by iret)
   
