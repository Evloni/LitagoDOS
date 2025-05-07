[BITS 16]           ; Tell NASM this is 16-bit code
[ORG 0x7C00]        ; BIOS loads bootloader at this address

; Set up segments
mov ax, 0x0000      ; Set up segments
mov ds, ax
mov es, ax
mov ss, ax
mov sp, 0x7C00      ; Set up stack pointer

; Print welcome message
mov si, welcome_msg ; Load message address
call print_string   ; Call print routine

; Infinite loop
jmp $               ; Jump to current location (infinite loop)

; Print string routine
print_string:
    mov ah, 0x0E    ; BIOS teletype output
.loop:
    lodsb           ; Load byte from SI into AL and increment SI
    test al, al     ; Check if character is null (end of string)
    jz .done        ; If zero, we're done
    int 0x10        ; Print character
    jmp .loop       ; Repeat for next character
.done:
    ret

; Data
welcome_msg: db 'Welcome to LitagoDOS!', 0x0D, 0x0A, 0

; Boot signature
times 510-($-$$) db 0   ; Pad to 510 bytes
dw 0xAA55               ; Boot signature 