global idt_load
extern idt_pointer

idt_load:
    lidt [idt_pointer]
    ret

; Interrupt handlers
global irq0
global irq1
; ... add more as needed

irq0:
    pusha
    ; Handle timer interrupt
    mov al, 0x20
    out 0x20, al
    popa
    iret

irq1:
    pusha
    ; Handle keyboard interrupt
    in al, 0x60    ; Read scancode
    ; Process scancode here
    mov al, 0x20
    out 0x20, al   ; Send EOI
    popa
    iret
