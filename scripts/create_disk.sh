#!/bin/bash

# Remove existing image if it exists
rm -f fat16.img

# Create and format a 16MB FAT16 disk image
mkfs.fat -F 16 -C fat16.img 16384

# Create a mount point
mkdir -p mnt

# Find an available loop device and set it up
LOOP_DEV=$(sudo losetup -f)
sudo losetup $LOOP_DEV fat16.img

# Mount the image
sudo mount -t vfat $LOOP_DEV mnt

# Create directory structure
sudo mkdir -p mnt/SYSTEM
sudo mkdir -p mnt/SYSTEM/FONTS
sudo mkdir -p mnt/DRIVERS
sudo mkdir -p mnt/APPS
sudo mkdir -p mnt/USER

# Create placeholder files
echo "Welcome to Litago!" | sudo tee mnt/USER/README.TXT

# Copy the BDF font file to the FONTS directory
sudo cp fonts/zap-light16.psf mnt/SYSTEM/FONTS/ZAPLIGHT.PSF

# Unmount the image
sudo umount mnt

# Detach the loop device
sudo losetup -d $LOOP_DEV

echo "Disk image created successfully with the specified file structure!"



