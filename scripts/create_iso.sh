#!/bin/bash

# Create ISO directory structure
mkdir -p isodir/boot/grub

# Copy kernel and filesystem
cp build/kernel.bin isodir/boot/
cp fat16.img isodir/boot/

# Create GRUB config
cat > isodir/boot/grub/grub.cfg << EOF
set timeout=0
set default=0

menuentry "LitagoOS" {
    multiboot /boot/kernel.bin
    module /boot/fat16.img
}
EOF

# Create bootable ISO
grub-mkrescue -o LitagoOS.iso isodir

echo "ISO created: LitagoOS.iso" 