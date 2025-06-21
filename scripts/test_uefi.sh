#!/bin/bash

set -e

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${GREEN}=== LitagoDOS UEFI QEMU Test ===${NC}"

# Check for UEFI binary
if [ ! -f build_uefi/BOOTX64.EFI ]; then
    echo -e "${RED}Error: build_uefi/BOOTX64.EFI not found. Build first!${NC}"
    exit 1
fi

# Create test disk image
IMG=build_uefi/test_disk.img
MNT=build_uefi/mnt
SIZE_MB=64

rm -f "$IMG"
mkdir -p "$MNT"

echo -e "${YELLOW}Creating FAT32 disk image...${NC}"
dd if=/dev/zero of="$IMG" bs=1M count=$SIZE_MB
mkfs.fat -F 32 "$IMG"

# Mount and copy UEFI binary
sudo mount -o loop "$IMG" "$MNT"
sudo mkdir -p "$MNT/EFI/BOOT"
sudo cp build_uefi/BOOTX64.EFI "$MNT/EFI/BOOT/BOOTX64.EFI"
sync
sudo umount "$MNT"

# Find OVMF firmware
OVMF_FD=""
for path in \
    /usr/share/ovmf/OVMF.fd \
    /usr/share/edk2/ovmf/OVMF.fd \
    /usr/share/qemu/ovmf-x86_64.bin \
    /usr/share/ovmf/x64/OVMF.fd; do
    if [ -f "$path" ]; then
        OVMF_FD="$path"
        break
    fi
done
if [ -z "$OVMF_FD" ]; then
    echo -e "${RED}Error: OVMF firmware not found. Install the ovmf package.${NC}"
    exit 1
fi

echo -e "${GREEN}Launching QEMU with OVMF...${NC}"
qemu-system-x86_64 \
    -bios "$OVMF_FD" \
    -drive file="$IMG",format=raw \
    -m 512 \
    -serial stdio \
    -display gtk \
    -no-reboot \
    -no-shutdown

echo -e "${GREEN}QEMU test completed.${NC}" 