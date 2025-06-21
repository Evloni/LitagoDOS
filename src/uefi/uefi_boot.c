#include "../include/uefi/uefi.h"
#include "../include/memory/uefi_memory.h"
#include <stddef.h>

void Print(EFI_SYSTEM_TABLE *SystemTable, uint16_t* str) {
    SystemTable->ConOut->OutputString(SystemTable->ConOut, str);
}

void ClearScreen(EFI_SYSTEM_TABLE *SystemTable) {
    SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
}

EFI_STATUS EFIAPI efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    // Clear the screen
    ClearScreen(SystemTable);

    Print(SystemTable, L"UEFI bootloader is running successfully!\n\r");
    Print(SystemTable, L"Initializing UEFI memory management...\n\r");
    
    // Initialize UEFI memory map
    uefi_memory_map_init(SystemTable);
    
    Print(SystemTable, L"Memory map initialized successfully!\n\r");
    
    // Print memory map information
    //uefi_memory_map_print(SystemTable);
    
    while(1);       
    // Wait for user input
    // SystemTable->BootServices->Stall(3000000); // 3 seconds
    
    // Exit boot services and halt
    // SystemTable->BootServices->Exit(ImageHandle, EFI_SUCCESS, 0, NULL);
    
    // This should never be reached, but just in case
    return EFI_SUCCESS;
}