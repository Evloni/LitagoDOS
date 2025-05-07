# Compiler and assembler settings
ASM = nasm
ASMFLAGS = -f bin

# Directories
SRC_DIR = src
BOOT_DIR = $(SRC_DIR)/bootloader
KERNEL_DIR = $(SRC_DIR)/kernel
BIN_DIR = bin

# Files
BOOT_BIN = $(BIN_DIR)/boot.bin
BOOT_ASM = $(BOOT_DIR)/boot.asm

# Default target
all:  $(BIN_DIR) $(BOOT_BIN) run clean

# Create bin directory
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Build bootloader
$(BOOT_BIN): $(BOOT_ASM) | $(BIN_DIR)
	$(ASM) $(ASMFLAGS) $< -o $@

# Clean build files
clean:
	rm -rf $(BIN_DIR)

# Run in QEMU
run: $(BOOT_BIN)
	qemu-system-i386 -drive format=raw,file=$(BOOT_BIN)

.PHONY: all clean run 