#include "../../include/memory/uefi_memory.h"
#include "../../include/uefi/uefi.h"
#include <stddef.h>

// Maximum number of memory map entries
#define MAX_UEFI_MEMORY_MAP_ENTRIES 64

// Static UEFI memory map
static struct uefi_memory_map uefi_memory_map = {
    .entries = NULL,
    .count = 0,
    .capacity = MAX_UEFI_MEMORY_MAP_ENTRIES,
    .map_key = 0,
    .descriptor_size = 0,
    .descriptor_version = 0
};

// Static array to store UEFI memory map entries
static EFI_MEMORY_DESCRIPTOR uefi_entries[MAX_UEFI_MEMORY_MAP_ENTRIES];

// UEFI memory types mapping
static const char* uefi_memory_type_names[] = {
    "Reserved",
    "Loader Code",
    "Loader Data", 
    "Boot Services Code",
    "Boot Services Data",
    "Runtime Services Code",
    "Runtime Services Data",
    "Conventional Memory",
    "Unusable Memory",
    "ACPI Reclaim Memory",
    "ACPI Memory NVS",
    "Memory Mapped I/O",
    "Memory Mapped I/O Port Space",
    "Pal Code",
    "Persistent Memory"
};

void uefi_memory_map_init(EFI_SYSTEM_TABLE* SystemTable) {
    EFI_STATUS status;
    UINTN map_size = 0;
    UINTN map_key = 0;
    UINTN descriptor_size = 0;
    UINT32 descriptor_version = 0;
    
    // Debug: Check if SystemTable and BootServices are valid
    if (SystemTable == NULL) {
        // We can't print here since we don't have SystemTable
        return;
    }
    
    if (SystemTable->BootServices == NULL) {
        return;
    }
    
    if (SystemTable->BootServices->GetMemoryMap == NULL) {
        return;
    }
    
    // First call to get the required buffer size
    status = SystemTable->BootServices->GetMemoryMap(
        &map_size,
        NULL,
        &map_key,
        &descriptor_size,
        &descriptor_version
    );
    
    if (status != EFI_BUFFER_TOO_SMALL) {
        // Handle error - we'll just initialize with empty map
        uefi_memory_map.count = 0;
        uefi_memory_map.entries = uefi_entries;
        return;
    }
    
    // Allocate buffer for memory map
    EFI_MEMORY_DESCRIPTOR* memory_map_buffer = NULL;
    // Add some padding to handle potential changes in memory map size
    UINTN allocated_size = map_size + 4096; // Add 4KB padding
    status = SystemTable->BootServices->AllocatePool(
        EfiLoaderData,
        allocated_size,
        (void**)&memory_map_buffer
    );
    
    if (EFI_ERROR(status) || memory_map_buffer == NULL) {
        uefi_memory_map.count = 0;
        uefi_memory_map.entries = uefi_entries;
        return;
    }
    
    // Get the actual memory map
    UINTN actual_map_size = allocated_size;
    status = SystemTable->BootServices->GetMemoryMap(
        &actual_map_size,
        memory_map_buffer,
        &map_key,
        &descriptor_size,
        &descriptor_version
    );
    
    if (EFI_ERROR(status)) {
        SystemTable->BootServices->FreePool(memory_map_buffer);
        uefi_memory_map.count = 0;
        uefi_memory_map.entries = uefi_entries;
        return;
    }
    
    // Copy memory map entries to our static array
    size_t entry_count = actual_map_size / descriptor_size;
    if (entry_count > MAX_UEFI_MEMORY_MAP_ENTRIES) {
        entry_count = MAX_UEFI_MEMORY_MAP_ENTRIES;
    }
    
    for (size_t i = 0; i < entry_count; i++) {
        EFI_MEMORY_DESCRIPTOR* src = (EFI_MEMORY_DESCRIPTOR*)((uint8_t*)memory_map_buffer + (i * descriptor_size));
        uefi_entries[i] = *src;
    }
    
    // Initialize our memory map structure
    uefi_memory_map.entries = uefi_entries;
    uefi_memory_map.count = entry_count;
    uefi_memory_map.map_key = map_key;
    uefi_memory_map.descriptor_size = descriptor_size;
    uefi_memory_map.descriptor_version = descriptor_version;
    
    // Free the temporary buffer
    SystemTable->BootServices->FreePool(memory_map_buffer);
}

uint64_t uefi_memory_map_get_total_memory(void) {
    uint64_t total = 0;
    
    for (size_t i = 0; i < uefi_memory_map.count; i++) {
        // Count conventional memory (type 7) and boot services data (type 4)
        if (uefi_memory_map.entries[i].Type == EfiConventionalMemory ||
            uefi_memory_map.entries[i].Type == EfiBootServicesData) {
            total += uefi_memory_map.entries[i].NumberOfPages * 4096; // Convert pages to bytes
        }
    }
    
    return total;
}

const struct uefi_memory_map* uefi_memory_map_get(void) {
    return &uefi_memory_map;
}

void uefi_memory_map_print(EFI_SYSTEM_TABLE* SystemTable) {
    if (SystemTable == NULL || SystemTable->ConOut == NULL) {
        return;
    }
    
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"\nUEFI Memory Map:\n");
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Base Address        Length              Type\n");
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"------------------------------------------------\n");
    
    for (size_t i = 0; i < uefi_memory_map.count; i++) {
        EFI_MEMORY_DESCRIPTOR* entry = &uefi_memory_map.entries[i];
        
        // Convert base address to hex string
        uint64_t base = entry->PhysicalStart;
        uint64_t length = entry->NumberOfPages * 4096; // Convert pages to bytes
        
        // Create hex string for base address
        wchar_t base_hex[17];
        for (int j = 15; j >= 0; j--) {
            base_hex[15-j] = L"0123456789ABCDEF"[(base >> (j * 4)) & 0xF];
        }
        base_hex[16] = L'\0';
        
        // Create hex string for length
        wchar_t length_hex[17];
        for (int j = 15; j >= 0; j--) {
            length_hex[15-j] = L"0123456789ABCDEF"[(length >> (j * 4)) & 0xF];
        }
        length_hex[16] = L'\0';
        
        // Print base address
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"0x");
        SystemTable->ConOut->OutputString(SystemTable->ConOut, base_hex);
        
        // Fixed spacing for alignment
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"  0x");
        SystemTable->ConOut->OutputString(SystemTable->ConOut, length_hex);
        
        // Print memory type with fixed-width formatting
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"  ");
        switch (entry->Type) {
            case EfiReservedMemoryType:
                SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Reserved");
                break;
            case EfiLoaderCode:
                SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Loader Code");
                break;
            case EfiLoaderData:
                SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Loader Data");
                break;
            case EfiBootServicesCode:
                SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Boot Services Code");
                break;
            case EfiBootServicesData:
                SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Boot Services Data");
                break;
            case EfiRuntimeServicesCode:
                SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Runtime Services Code");
                break;
            case EfiRuntimeServicesData:
                SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Runtime Services Data");
                break;
            case EfiConventionalMemory:
                SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Conventional Memory");
                break;
            case EfiUnusableMemory:
                SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Unusable Memory");
                break;
            case EfiACPIReclaimMemory:
                SystemTable->ConOut->OutputString(SystemTable->ConOut, L"ACPI Reclaim Memory");
                break;
            case EfiACPIMemoryNVS:
                SystemTable->ConOut->OutputString(SystemTable->ConOut, L"ACPI Memory NVS");
                break;
            case EfiMemoryMappedIO:
                SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Memory Mapped I/O");
                break;
            case EfiMemoryMappedIOPortSpace:
                SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Memory Mapped I/O Port Space");
                break;
            case EfiPalCode:
                SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Pal Code");
                break;
            case EfiPersistentMemory:
                SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Persistent Memory");
                break;
            default:
                SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Unknown");
                break;
        }
        
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"\n");
    }
    
    // Print total usable memory
    uint64_t total = uefi_memory_map_get_total_memory();
    uint64_t total_mb = total / (1024 * 1024);
    
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"\nTotal Usable Memory: ");
    
    // Convert to string
    wchar_t mb_str[20];
    int i = 0;
    uint64_t temp_mb = total_mb;
    
    do {
        mb_str[i++] = L'0' + (temp_mb % 10);
        temp_mb /= 10;
    } while (temp_mb > 0);
    
    // Print in reverse
    while (--i >= 0) {
        wchar_t digit[2] = {mb_str[i], L'\0'};
        SystemTable->ConOut->OutputString(SystemTable->ConOut, digit);
    }
    
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L" MB\n\n");
}

EFI_STATUS uefi_exit_boot_services(EFI_SYSTEM_TABLE* SystemTable, EFI_HANDLE ImageHandle) {
    EFI_STATUS status;
    UINTN map_size = 0;
    UINTN map_key = 0;
    UINTN descriptor_size = 0;
    UINT32 descriptor_version = 0;
    
    // Get the final memory map before exiting boot services
    status = SystemTable->BootServices->GetMemoryMap(
        &map_size,
        NULL,
        &map_key,
        &descriptor_size,
        &descriptor_version
    );
    
    if (status != EFI_BUFFER_TOO_SMALL) {
        return status;
    }
    
    // Allocate buffer for final memory map
    EFI_MEMORY_DESCRIPTOR* final_memory_map = NULL;
    status = SystemTable->BootServices->AllocatePool(
        EfiLoaderData,
        map_size,
        (void**)&final_memory_map
    );
    
    if (EFI_ERROR(status)) {
        return status;
    }
    
    // Get the final memory map
    status = SystemTable->BootServices->GetMemoryMap(
        &map_size,
        final_memory_map,
        &map_key,
        &descriptor_size,
        &descriptor_version
    );
    
    if (EFI_ERROR(status)) {
        SystemTable->BootServices->FreePool(final_memory_map);
        return status;
    }
    
    // Exit boot services
    status = SystemTable->BootServices->ExitBootServices(ImageHandle, map_key);
    
    if (EFI_ERROR(status)) {
        SystemTable->BootServices->FreePool(final_memory_map);
        return status;
    }
    
    // Store the final memory map for kernel use
    // Note: In a real implementation, you'd want to store this in a global variable
    // or pass it to your kernel initialization function
    
    return EFI_SUCCESS;
} 