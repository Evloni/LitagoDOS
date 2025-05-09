#!/bin/bash

# Create a 16MB FAT16 disk image (32768 sectors)
dd if=/dev/zero of=fat16.img bs=512 count=32768

# Format it as FAT16
mkfs.fat -F 16 -n "LITAGO" fat16.img

# Create some test files
mkdir -p mnt
sudo mount -o loop fat16.img mnt
sudo bash -c 'echo "Hello, World!" > mnt/test.txt'
sudo bash -c 'echo "This is a test file" > mnt/readme.txt'
sudo umount mnt
rm -rf mnt

echo "Created FAT16 disk image: fat16.img" 