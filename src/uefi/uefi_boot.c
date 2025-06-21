#include "../include/uefi/uefi.h"
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
    while(1);

    // Wait a bit so user can see the message
    SystemTable->BootServices->Stall(3000000); // 3 seconds

    // Exit boot services and halt
    SystemTable->BootServices->Exit(ImageHandle, EFI_SUCCESS, 0, NULL);
    
    // This should never be reached, but just in case
    return EFI_SUCCESS;
}