# Compiler and linker settings
ASM = nasm
CC = gcc
LD = ld

# Emulator settings
QEMU = qemu-system-i386
QEMU_FLAGS = -machine q35 -m 128M -no-reboot -no-shutdown

# Flags
ASMFLAGS = -f elf32
CFLAGS = -m32 -ffreestanding -fno-pie -fno-stack-protector -nostdlib -c -Iinclude
LDFLAGS = -m elf_i386 -T linker.ld -nostdlib

# Directories
BUILD_DIR = build
ISO_DIR = isodir
ISO_BOOT_DIR = $(ISO_DIR)/boot
ISO_GRUB_DIR = $(ISO_DIR)/boot/grub
SRC_DIR = src
ASM_DIR = asm
INCLUDE_DIR = include

# Files
KERNEL_ENTRY_ASM = $(ASM_DIR)/kernel_entry.asm
KERNEL_C = $(SRC_DIR)/kernel.c
VGA_C = $(SRC_DIR)/vga.c
IO_C = $(SRC_DIR)/io.c
LINK_SCRIPT = linker.ld
KERNEL_ENTRY_OBJ = $(BUILD_DIR)/kernel_entry.o
KERNEL_OBJ = $(BUILD_DIR)/kernel.o
VGA_OBJ = $(BUILD_DIR)/vga.o
IO_OBJ = $(BUILD_DIR)/io.o
KERNEL_BIN = $(BUILD_DIR)/kernel.bin
ISO_IMAGE = $(BUILD_DIR)/Litago.iso

# Default target
.PHONY: all
all: $(ISO_IMAGE) run clean

# Create build directories
$(BUILD_DIR):
	mkdir -p $@
	mkdir -p $(ISO_BOOT_DIR)
	mkdir -p $(ISO_GRUB_DIR)

# Assemble kernel entry
$(KERNEL_ENTRY_OBJ): $(KERNEL_ENTRY_ASM) | $(BUILD_DIR)
	@echo "Assembling $<..."
	$(ASM) $(ASMFLAGS) $< -o $@

# Compile kernel
$(KERNEL_OBJ): $(KERNEL_C) | $(BUILD_DIR)
	@echo "Compiling kernel..."
	$(CC) $(CFLAGS) $< -o $@

# Compile VGA
$(VGA_OBJ): $(VGA_C) | $(BUILD_DIR)
	@echo "Compiling VGA..."
	$(CC) $(CFLAGS) $< -o $@

# Compile I/O
$(IO_OBJ): $(IO_C) | $(BUILD_DIR)
	@echo "Compiling I/O..."
	$(CC) $(CFLAGS) $< -o $@

# Link kernel
$(KERNEL_BIN): $(KERNEL_ENTRY_OBJ) $(KERNEL_OBJ) $(VGA_OBJ) $(IO_OBJ) $(LINK_SCRIPT)
	@echo "Linking kernel..."
	$(LD) $(LDFLAGS) $(KERNEL_ENTRY_OBJ) $(KERNEL_OBJ) $(VGA_OBJ) $(IO_OBJ) -o $@

# Create ISO image
$(ISO_IMAGE): $(KERNEL_BIN) | $(BUILD_DIR)
	@echo "Creating ISO image..."
	cp $(KERNEL_BIN) $(ISO_BOOT_DIR)/kernel.bin
	cp grub.cfg $(ISO_GRUB_DIR)/
	grub-mkrescue -o $@ $(ISO_DIR)

# Run the OS in QEMU
.PHONY: run
run: $(ISO_IMAGE)
	@echo "Starting QEMU..."
	$(QEMU) $(QEMU_FLAGS) -cdrom $(ISO_IMAGE)

# Clean build files
.PHONY: clean
clean:
	@echo "Cleaning build files..."
	rm -rf $(BUILD_DIR)
	rm -rf $(ISO_DIR)

# Help target
.PHONY: help
help:
	@echo "Available targets:"
	@echo "  all     - Build the OS and create ISO image"
	@echo "  run     - Run the OS in QEMU"
	@echo "  clean   - Remove all build files"
	@echo "  help    - Show this help message"
