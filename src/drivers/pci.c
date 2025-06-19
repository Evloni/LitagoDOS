#include "../include/drivers/pci.h"
#include <stddef.h>

typedef struct {
    uint16_t vendor_id;
    uint16_t device_id;
    const char* vendor_name;
    const char* device_name;
} pci_device_t;

static const pci_device_t pci_devices[] = {
    {0x8086, 0x1237, "Intel Corporation", "440FX - 82441FX PMC [Natoma]"},
    {0x8086, 0x7000, "Intel Corporation", "82371SB PIIX3 ISA [Natoma/Triton II]"},
    {0x8086, 0x7010, "Intel Corporation", "82371SB PIIX3 ISA [Natoma/Triton II]"},
    {0x8086, 0x7113, "Intel Corporation", "82371AB/EB/MB PIIX4 ACPI"},
    {0x8086, 0x100E, "Intel Corporation", "82540EM Gigabit Ethernet Controller"},
    {0x1234, 0x1111, "Bochs", "VGA Compatible Graphics Adapter"},

};

const pci_device_t* find_pci_device(uint16_t vendor, uint16_t device) {
    for (int i = 0; pci_devices[i].vendor_name; i++) {
        if (pci_devices[i].vendor_id == vendor && pci_devices[i].device_id == device) {
            return &pci_devices[i];
        }
    }
    return NULL;
}




uint32_t pci_config_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset){
    uint32_t address;
    address = (uint32_t)((bus << 16) | (slot<< 11) | (func <<8) | (offset & 0xFC) | ((uint32_t)0x80000000));
    outl(0xCF8, address);
    return inl(0xCFC);
}
void pci_scan() {
    for (int bus = 0; bus < 256; bus++) {
        for (int device = 0; device < 32; device++) {
            for (int function = 0; function < 8; function++) {
                uint16_t vendor = pci_config_read(bus, device, function, 0x00) & 0xFFFF;
                if (vendor != 0xFFFF) {
                    uint16_t device_id = (pci_config_read(bus, device, function, 0x00) >> 16) & 0xFFFF;
                    const pci_device_t* info = find_pci_device(vendor, device_id);
                    if (info) {
                        printf("PCI Device: %s %s at %d:%d.0\n", info->vendor_name, info->device_name, bus, device);
                    } else {
                        printf("PCI Device: %04x:%04x at %d:%d.0\n", vendor, device_id, bus, device);
                    }
                }
            }
        }
    }
}
