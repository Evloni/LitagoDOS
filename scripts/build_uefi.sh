#!/bin/bash

# UEFI Build Script for LitagoDOS
# This script builds a basic UEFI application

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== LitagoDOS UEFI Build Script ===${NC}"

# Check if we're in the right directory
if [ ! -f "Makefile" ]; then
    echo -e "${RED}Error: Makefile not found. Please run this script from the project root.${NC}"
    exit 1
fi

# Create build directory
BUILD_DIR="build_uefi"
mkdir -p "$BUILD_DIR"

echo -e "${YELLOW}Building UEFI application...${NC}"

# Check for required tools
if ! command -v x86_64-w64-mingw32-gcc &> /dev/null; then
    echo -e "${RED}Error: x86_64-w64-mingw32-gcc not found. Please install mingw-w64.${NC}"
    echo -e "${YELLOW}On Ubuntu/Debian: sudo apt install mingw-w64${NC}"
    exit 1
fi

if ! command -v x86_64-w64-mingw32-objcopy &> /dev/null; then
    echo -e "${RED}Error: x86_64-w64-mingw32-objcopy not found. Please install mingw-w64.${NC}"
    exit 1
fi

# Compiler settings for UEFI
CC="x86_64-w64-mingw32-gcc"
CFLAGS="-m64 -ffreestanding -fno-stack-protector -nostdlib -c -Iinclude -mno-red-zone -fno-exceptions -O0 -D__UEFI__"
LDFLAGS="-nostdlib -Wl,--subsystem,10 -Wl,-entry,efi_main -Wl,-image-base,0x200000"

# Source files for UEFI
UEFI_SOURCES=(
    "src/uefi/uefi_boot.c"
    "src/memory/uefi_memory.c"
)

# Object files
UEFI_OBJECTS=()

# Compile UEFI sources
for source in "${UEFI_SOURCES[@]}"; do
    if [ -f "$source" ]; then
        obj_file="$BUILD_DIR/$(basename "$source" .c).o"
        echo "Compiling $source -> $obj_file"
        $CC $CFLAGS "$source" -o "$obj_file"
        UEFI_OBJECTS+=("$obj_file")
    else
        echo -e "${RED}Error: Source file $source not found${NC}"
        exit 1
    fi
done

# Link to PE/COFF format
UEFI_PE="$BUILD_DIR/LitagoDOS.pe"
echo "Linking to PE/COFF format..."
$CC $LDFLAGS -o "$UEFI_PE" "${UEFI_OBJECTS[@]}"

# Convert to proper UEFI PE/COFF format
UEFI_EFI="$BUILD_DIR/LitagoDOS.efi"
echo "Converting to UEFI PE/COFF format..."
x86_64-w64-mingw32-objcopy \
    --target=efi-app-x86-64 \
    --subsystem=10 \
    "$UEFI_PE" \
    "$UEFI_EFI"

# Create bootable UEFI image
echo "Creating bootable UEFI image..."
UEFI_BOOT="$BUILD_DIR/BOOTX64.EFI"
cp "$UEFI_EFI" "$UEFI_BOOT"

echo -e "${GREEN}UEFI build completed successfully!${NC}"
echo -e "${GREEN}Output files:${NC}"
echo -e "  - $UEFI_EFI (UEFI PE/COFF application)"
echo -e "  - $UEFI_BOOT (Bootable UEFI image)"

# Check if files were created
if [ -f "$UEFI_EFI" ]; then
    echo -e "${GREEN}✓ UEFI application created successfully${NC}"
    ls -la "$UEFI_EFI"
    
    # Show file type information
    echo -e "${YELLOW}File information:${NC}"
    file "$UEFI_EFI"
else
    echo -e "${RED}✗ Failed to create UEFI application${NC}"
    exit 1
fi

echo -e "${YELLOW}Next steps:${NC}"
echo -e "1. Use scripts/test_uefi.sh to create a disk image and run in QEMU." 