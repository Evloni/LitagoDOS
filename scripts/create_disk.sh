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
sudo mkdir -p mnt/DRIVERS
sudo mkdir -p mnt/APPS
sudo mkdir -p mnt/USER
sudo mkdir -p mnt/FONTS

# Create placeholder files
echo "Welcome to LitagoDOS!" | sudo tee mnt/USER/README.TXT
echo "Hello World!" | sudo tee mnt/USER/HELLOWORLD.TXT

# Copy the BDF font file to the FONTS directory
sudo cp fonts/unifont-16.0.04.bdf mnt/FONT.BDF

# Unmount the image
sudo umount mnt

# Detach the loop device
sudo losetup -d $LOOP_DEV

echo "Disk image created successfully with the specified file structure!"



