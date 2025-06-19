#include <stdint.h>
#include "../include/io.h"
#include "../include/stdio.h"

uint32_t pci_config_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
void pci_scan();