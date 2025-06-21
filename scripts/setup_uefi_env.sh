#!/bin/bash

# UEFI Development Environment Setup Script
# This script installs the necessary tools for UEFI development

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== LitagoDOS UEFI Development Environment Setup ===${NC}"

# Check if running as root (needed for package installation)
if [ "$EUID" -eq 0 ]; then
    echo -e "${RED}Error: Please don't run this script as root.${NC}"
    exit 1
fi

# Detect package manager
if command -v apt-get &> /dev/null; then
    PKG_MANAGER="apt-get"
    UPDATE_CMD="sudo apt-get update"
    INSTALL_CMD="sudo apt-get install -y"
elif command -v yum &> /dev/null; then
    PKG_MANAGER="yum"
    UPDATE_CMD="sudo yum update -y"
    INSTALL_CMD="sudo yum install -y"
elif command -v dnf &> /dev/null; then
    PKG_MANAGER="dnf"
    UPDATE_CMD="sudo dnf update -y"
    INSTALL_CMD="sudo dnf install -y"
elif command -v pacman &> /dev/null; then
    PKG_MANAGER="pacman"
    UPDATE_CMD="sudo pacman -Sy"
    INSTALL_CMD="sudo pacman -S --noconfirm"
else
    echo -e "${RED}Error: Unsupported package manager. Please install the required packages manually.${NC}"
    exit 1
fi

echo -e "${BLUE}Detected package manager: $PKG_MANAGER${NC}"

# Update package list
echo -e "${YELLOW}Updating package list...${NC}"
$UPDATE_CMD

# Install required packages
echo -e "${YELLOW}Installing UEFI development tools...${NC}"

if [ "$PKG_MANAGER" = "apt-get" ]; then
    # Ubuntu/Debian packages
    $INSTALL_CMD build-essential gcc-multilib g++-multilib
    $INSTALL_CMD mingw-w64 gcc-mingw-w64 g++-mingw-w64
    $INSTALL_CMD qemu-system-x86 ovmf
    $INSTALL_CMD mtools dosfstools
    $INSTALL_CMD uuid-runtime
elif [ "$PKG_MANAGER" = "yum" ] || [ "$PKG_MANAGER" = "dnf" ]; then
    # Fedora/RHEL/CentOS packages
    $INSTALL_CMD gcc gcc-c++ glibc-devel
    $INSTALL_CMD mingw64-gcc mingw64-gcc-c++
    $INSTALL_CMD qemu-system-x86 edk2-ovmf
    $INSTALL_CMD mtools dosfstools
    $INSTALL_CMD uuid
elif [ "$PKG_MANAGER" = "pacman" ]; then
    # Arch Linux packages
    $INSTALL_CMD base-devel
    $INSTALL_CMD mingw-w64-gcc
    $INSTALL_CMD qemu ovmf
    $INSTALL_CMD mtools dosfstools
    $INSTALL_CMD util-linux
fi

echo -e "${GREEN}✓ UEFI development tools installed successfully${NC}"

# Check if tools are available
echo -e "${YELLOW}Verifying installation...${NC}"

TOOLS=(
    "x86_64-w64-mingw32-gcc"
    "x86_64-w64-mingw32-objcopy"
    "qemu-system-x86_64"
)

for tool in "${TOOLS[@]}"; do
    if command -v "$tool" &> /dev/null; then
        echo -e "${GREEN}✓ $tool is available${NC}"
    else
        echo -e "${RED}✗ $tool is not available${NC}"
        echo -e "${YELLOW}  You may need to install it manually or add it to your PATH${NC}"
    fi
done

# Create test UEFI build
echo -e "${YELLOW}Testing UEFI build...${NC}"
if [ -f "scripts/build_uefi.sh" ]; then
    echo -e "${BLUE}Running UEFI build test...${NC}"
    if ./scripts/build_uefi.sh; then
        echo -e "${GREEN}✓ UEFI build test successful${NC}"
    else
        echo -e "${RED}✗ UEFI build test failed${NC}"
        echo -e "${YELLOW}  Check the error messages above for details${NC}"
    fi
else
    echo -e "${YELLOW}UEFI build script not found, skipping test${NC}"
fi

# Create QEMU test script
echo -e "${YELLOW}Creating QEMU test script...${NC}"
cat > scripts/test_uefi.sh << 'EOF'
#!/bin/bash

# QEMU UEFI Test Script for LitagoDOS

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${GREEN}=== LitagoDOS UEFI QEMU Test ===${NC}"

# Check if UEFI build exists
UEFI_EFI="build_uefi/LitagoDOS.efi"
if [ ! -f "$UEFI_EFI" ]; then
    echo -e "${RED}Error: UEFI application not found. Please run build_uefi.sh first.${NC}"
    exit 1
fi

# Create test disk image
echo -e "${YELLOW}Creating test disk image...${NC}"
TEST_IMG="build_uefi/test_disk.img"
mkdir -p build_uefi/efi/boot

# Copy UEFI application
cp "$UEFI_EFI" build_uefi/efi/boot/BOOTX64.EFI

# Create FAT32 disk image
dd if=/dev/zero of="$TEST_IMG" bs=1M count=64
mkfs.fat -F 32 "$TEST_IMG"
mcopy -i "$TEST_IMG" -s build_uefi/efi ::

echo -e "${GREEN}Test disk image created: $TEST_IMG${NC}"

# Find OVMF firmware
OVMF_PATHS=(
    "/usr/share/ovmf/OVMF.fd"
    "/usr/share/edk2/ovmf/OVMF.fd"
    "/usr/share/qemu/ovmf-x86_64.bin"
    "/usr/share/ovmf/x64/OVMF.fd"
)

OVMF_FD=""
for path in "${OVMF_PATHS[@]}"; do
    if [ -f "$path" ]; then
        OVMF_FD="$path"
        break
    fi
done

if [ -z "$OVMF_FD" ]; then
    echo -e "${RED}Error: OVMF firmware not found. Please install edk2-ovmf package.${NC}"
    exit 1
fi

echo -e "${GREEN}Using OVMF firmware: $OVMF_FD${NC}"

# Run QEMU
echo -e "${YELLOW}Starting QEMU with UEFI...${NC}"
echo -e "${BLUE}Press Ctrl+C to stop QEMU${NC}"

qemu-system-x86_64 \
    -bios "$OVMF_FD" \
    -hda "$TEST_IMG" \
    -m 512 \
    -serial stdio \
    -display gtk \
    -no-reboot \
    -no-shutdown

echo -e "${GREEN}QEMU test completed${NC}"
EOF

chmod +x scripts/test_uefi.sh

echo -e "${GREEN}✓ QEMU test script created: scripts/test_uefi.sh${NC}"

# Create USB boot script
echo -e "${YELLOW}Creating USB boot script...${NC}"
cat > scripts/create_uefi_usb.sh << 'EOF'
#!/bin/bash

# Create UEFI Bootable USB Script for LitagoDOS

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${GREEN}=== LitagoDOS UEFI USB Boot Creator ===${NC}"

# Check if UEFI build exists
UEFI_EFI="build_uefi/LitagoDOS.efi"
if [ ! -f "$UEFI_EFI" ]; then
    echo -e "${RED}Error: UEFI application not found. Please run build_uefi.sh first.${NC}"
    exit 1
fi

# Check if device is provided
if [ $# -eq 0 ]; then
    echo -e "${YELLOW}Usage: $0 <device>${NC}"
    echo -e "${YELLOW}Example: $0 /dev/sdb${NC}"
    echo -e "${RED}Warning: This will format the device! Make sure you have backups!${NC}"
    exit 1
fi

DEVICE="$1"

# Check if device exists
if [ ! -b "$DEVICE" ]; then
    echo -e "${RED}Error: Device $DEVICE not found${NC}"
    exit 1
fi

# Confirm before proceeding
echo -e "${RED}WARNING: This will format $DEVICE and destroy all data!${NC}"
echo -e "${YELLOW}Are you sure you want to continue? (y/N)${NC}"
read -r response
if [[ ! "$response" =~ ^[Yy]$ ]]; then
    echo -e "${YELLOW}Operation cancelled${NC}"
    exit 1
fi

echo -e "${YELLOW}Creating UEFI bootable USB on $DEVICE...${NC}"

# Unmount device if mounted
if mount | grep -q "$DEVICE"; then
    echo "Unmounting $DEVICE..."
    sudo umount "$DEVICE"* 2>/dev/null || true
fi

# Create FAT32 filesystem
echo "Creating FAT32 filesystem..."
sudo mkfs.fat -F 32 "$DEVICE"

# Create mount point
MOUNT_POINT="/tmp/litagodos_uefi_mount"
sudo mkdir -p "$MOUNT_POINT"

# Mount device
echo "Mounting device..."
sudo mount "$DEVICE" "$MOUNT_POINT"

# Create EFI directory structure
echo "Creating EFI directory structure..."
sudo mkdir -p "$MOUNT_POINT/EFI/BOOT"

# Copy UEFI application
echo "Copying UEFI application..."
sudo cp "$UEFI_EFI" "$MOUNT_POINT/EFI/BOOT/BOOTX64.EFI"

# Unmount device
echo "Unmounting device..."
sudo umount "$MOUNT_POINT"
sudo rmdir "$MOUNT_POINT"

echo -e "${GREEN}✓ UEFI bootable USB created successfully!${NC}"
echo -e "${GREEN}Device: $DEVICE${NC}"
echo -e "${YELLOW}Next steps:${NC}"
echo -e "1. Boot your computer in UEFI mode"
echo -e "2. Select the USB device as boot option"
echo -e "3. LitagoDOS should start automatically"
EOF

chmod +x scripts/create_uefi_usb.sh

echo -e "${GREEN}✓ USB boot script created: scripts/create_uefi_usb.sh${NC}"

echo -e "${GREEN}=== UEFI Development Environment Setup Complete ===${NC}"
echo -e "${YELLOW}Available scripts:${NC}"
echo -e "  - scripts/build_uefi.sh     - Build UEFI application"
echo -e "  - scripts/test_uefi.sh      - Test with QEMU"
echo -e "  - scripts/create_uefi_usb.sh - Create bootable USB"
echo -e ""
echo -e "${YELLOW}Next steps:${NC}"
echo -e "1. Run: ./scripts/build_uefi.sh"
echo -e "2. Test with: ./scripts/test_uefi.sh"
echo -e "3. Create USB with: ./scripts/create_uefi_usb.sh <device>" 