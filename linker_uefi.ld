/* UEFI 64-bit Linker Script for LitagoDOS */

OUTPUT_FORMAT("elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
ENTRY(efi_main)

SECTIONS
{
    /* UEFI applications are loaded at 0x200000 */
    . = 0x200000;
    
    /* PE/COFF header will be added by objcopy */
    .text : {
        *(.text)
        *(.text.*)
    }
    
    .rodata : {
        *(.rodata)
        *(.rodata.*)
    }
    
    .data : {
        *(.data)
        *(.data.*)
    }
    
    .bss : {
        *(.bss)
        *(.bss.*)
        *(COMMON)
    }
    
    /* Discard other sections */
    /DISCARD/ : {
        *(.comment)
        *(.note)
        *(.eh_frame)
        *(.eh_frame_hdr)
    }
} 