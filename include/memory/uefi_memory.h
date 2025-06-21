#ifndef UEFI_MEMORY_H
#define UEFI_MEMORY_H

#include <stdint.h>
#include <stddef.h>
#include "../uefi/uefi.h"

// UEFI memory types (from UEFI specification)
#define EfiReservedMemoryType          0
#define EfiLoaderCode                  1
#define EfiLoaderData                  2
#define EfiBootServicesCode            3
#define EfiBootServicesData            4
#define EfiRuntimeServicesCode         5
#define EfiRuntimeServicesData         6
#define EfiConventionalMemory          7
#define EfiUnusableMemory              8
#define EfiACPIReclaimMemory           9
#define EfiACPIMemoryNVS               10
#define EfiMemoryMappedIO              11
#define EfiMemoryMappedIOPortSpace     12
#define EfiPalCode                     13
#define EfiPersistentMemory            14

// UEFI memory map structure
struct uefi_memory_map {
    EFI_MEMORY_DESCRIPTOR* entries;
    size_t count;
    size_t capacity;
    UINTN map_key;
    UINTN descriptor_size;
    UINT32 descriptor_version;
};

// Initialize UEFI memory map from UEFI system table
void uefi_memory_map_init(EFI_SYSTEM_TABLE* SystemTable);

// Get total usable memory size
uint64_t uefi_memory_map_get_total_memory(void);

// Get UEFI memory map
const struct uefi_memory_map* uefi_memory_map_get(void);

// Print UEFI memory map information
void uefi_memory_map_print(EFI_SYSTEM_TABLE* SystemTable);

// Exit UEFI boot services and get final memory map
EFI_STATUS uefi_exit_boot_services(EFI_SYSTEM_TABLE* SystemTable, EFI_HANDLE ImageHandle);

#endif // UEFI_MEMORY_H 