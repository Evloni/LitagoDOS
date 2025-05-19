#!/bin/bash

# test_iso_fs.sh

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}Testing ISO filesystem driver...${NC}"

# Create a test filesystem
echo -e "${YELLOW}Creating test filesystem...${NC}"
./scripts/create_disk.sh

# Build the OS
echo -e "${YELLOW}Building OS...${NC}"
make clean
make

# Run in QEMU
echo -e "${YELLOW}Running in QEMU...${NC}"
qemu-system-i386 -machine pc -m 2G -cdrom build/Litago.iso -hda fat16.img -boot d 