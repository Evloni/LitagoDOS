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

# Create some test files
echo "Hello, World!" | sudo tee mnt/test.txt
echo "This is a test file" | sudo tee mnt/readme.txt

# Unmount the image
sudo umount mnt

# Detach the loop device
sudo losetup -d $LOOP_DEV

echo "Disk image created successfully!"