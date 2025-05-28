# Compiler and linker settings
ASM = nasm
CC = gcc
LD = ld


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
BOOT_ASM = $(SRC_DIR)/boot.asm
KERNEL_C = $(SRC_DIR)/kernel.c
VGA_C = $(SRC_DIR)/vga.c
IO_C = $(SRC_DIR)/io.c
LINK_SCRIPT = linker.ld
BOOT_OBJ = $(BUILD_DIR)/boot.o
KERNEL_OBJ = $(BUILD_DIR)/kernel.o
VGA_OBJ = $(BUILD_DIR)/vga.o
IO_OBJ = $(BUILD_DIR)/io.o
KERNEL_BIN = $(BUILD_DIR)/kernel.bin
ISO_IMAGE = $(BUILD_DIR)/Litago.iso
IDT_ASM = $(SRC_DIR)/interrupts/idt.asm
IDT_C = $(SRC_DIR)/interrupts/idt.c
GDT_C = $(SRC_DIR)/interrupts/gdt.c
IDT_ASM_OBJ = $(BUILD_DIR)/idt.o
IDT_C_OBJ = $(BUILD_DIR)/idt_c.o
GDT_C_OBJ = $(BUILD_DIR)/gdt.o

# Add these new variables after your existing file definitions
DRIVERS_DIR = $(SRC_DIR)/drivers
VGA_DRIVER_C = $(DRIVERS_DIR)/vga_driver.c
VGA_DRIVER_OBJ = $(BUILD_DIR)/vga_driver.o
KEYBOARD_DRIVER_C = $(DRIVERS_DIR)/keyboardDriver.c
KEYBOARD_DRIVER_OBJ = $(BUILD_DIR)/keyboardDriver.o
TIMER_DRIVER_C = $(DRIVERS_DIR)/timerDriver.c
TIMER_DRIVER_OBJ = $(BUILD_DIR)/timerDriver.o
STRING_C = $(SRC_DIR)/string.c
STRING_OBJ = $(BUILD_DIR)/string.o
SHELL_C = $(SRC_DIR)/shell/shell.c
SHELL_OBJ = $(BUILD_DIR)/shell.o

# Memory management files
PMM_C = $(SRC_DIR)/memory/pmm.c
PMM_OBJ = $(BUILD_DIR)/pmm.o
MEMORY_MAP_C = $(SRC_DIR)/memory/memory_map.c
MEMORY_MAP_OBJ = $(BUILD_DIR)/memory_map.o
HEAP_C = $(SRC_DIR)/memory/heap.c
HEAP_OBJ = $(BUILD_DIR)/heap.o
STDLIB_C = $(SRC_DIR)/memory/stdlib.c
STDLIB_OBJ = $(BUILD_DIR)/stdlib.o
PROGRAM_C = $(SRC_DIR)/memory/program.c
PROGRAM_OBJ = $(BUILD_DIR)/program.o

# Library files
LIBGCC_C = $(SRC_DIR)/libgcc.c
LIBGCC_OBJ = $(BUILD_DIR)/libgcc.o

# Test source files
TEST_C = $(SRC_DIR)/tests/memtest.c
TEST_OBJ = $(BUILD_DIR)/tests/memtest.o
SYSCALL_TEST_C = $(SRC_DIR)/tests/syscall_test.c
SYSCALL_TEST_OBJ = $(BUILD_DIR)/tests/syscall_test.o

# Add test.c for shell memtest2
TEST2_C = $(SRC_DIR)/test.c
TEST2_OBJ = $(BUILD_DIR)/test2.o

# Add after your other file definitions
SYSCALL_ASM = $(SRC_DIR)/interrupts/syscall.asm
SYSCALL_ASM_OBJ = $(BUILD_DIR)/syscall.o
SYSCALL_C = $(SRC_DIR)/syscall/syscall.c
SYSCALL_C_OBJ = $(BUILD_DIR)/syscall_c.o

# ANSI support files
ANSI_C = $(SRC_DIR)/ansi.c
ANSI_OBJ = $(BUILD_DIR)/ansi.o

# Version files
VERSION_C = $(SRC_DIR)/version.c
VERSION_OBJ = $(BUILD_DIR)/version.o

# Filesystem files
FS_DIR = $(SRC_DIR)/fs
FAT16_C = $(FS_DIR)/fat16.c
FAT16_OBJ = $(BUILD_DIR)/fat16.o

# ATA driver files
ATA_C = $(SRC_DIR)/drivers/ata.c
ATA_OBJ = $(BUILD_DIR)/ata.o

# Editor files
EDITOR_C = $(SRC_DIR)/editor.c
EDITOR_OBJ = $(BUILD_DIR)/editor.o

# ISO filesystem driver files
ISO_FS_C = $(DRIVERS_DIR)/iso_fs.c
ISO_FS_OBJ = $(BUILD_DIR)/iso_fs.o

# ISO filesystem test
ISO_FS_TEST_C = $(SRC_DIR)/tests/iso_fs_test.c
ISO_FS_TEST_OBJ = $(BUILD_DIR)/tests/iso_fs_test.o

# Add ISO_FS_OBJ and ISO_FS_TEST_OBJ to the OBJS list
OBJS = $(BOOT_OBJ) $(KERNEL_OBJ) $(VGA_OBJ) $(IO_OBJ) $(IDT_ASM_OBJ) $(IDT_C_OBJ) \
       $(GDT_C_OBJ) $(VGA_DRIVER_OBJ) $(KEYBOARD_DRIVER_OBJ) $(TIMER_DRIVER_OBJ) \
       $(STRING_OBJ) $(SHELL_OBJ) $(PMM_OBJ) $(MEMORY_MAP_OBJ) $(HEAP_OBJ) $(STDLIB_OBJ) $(LIBGCC_OBJ) \
       $(TEST_OBJ) $(TEST2_OBJ) $(SYSCALL_TEST_OBJ) $(SYSCALL_ASM_OBJ) $(SYSCALL_C_OBJ) \
       $(VERSION_OBJ) $(FAT16_OBJ) $(ATA_OBJ) $(PROGRAM_OBJ) $(EDITOR_OBJ) $(ISO_FS_OBJ) $(ISO_FS_TEST_OBJ) \
       $(ANSI_OBJ)

# Add after your other file definitions
UTILS_DIR = $(SRC_DIR)/utils
PROGRESS_C = $(UTILS_DIR)/progress.c
PROGRESS_OBJ = $(BUILD_DIR)/progress.o

# Add ISO_FS_OBJ and ISO_FS_TEST_OBJ to the OBJS list
OBJS = $(BOOT_OBJ) $(KERNEL_OBJ) $(VGA_OBJ) $(IO_OBJ) $(IDT_ASM_OBJ) $(IDT_C_OBJ) \
       $(GDT_C_OBJ) $(VGA_DRIVER_OBJ) $(KEYBOARD_DRIVER_OBJ) $(TIMER_DRIVER_OBJ) \
       $(STRING_OBJ) $(SHELL_OBJ) $(PMM_OBJ) $(MEMORY_MAP_OBJ) $(HEAP_OBJ) $(STDLIB_OBJ) $(LIBGCC_OBJ) \
       $(TEST_OBJ) $(TEST2_OBJ) $(SYSCALL_TEST_OBJ) $(SYSCALL_ASM_OBJ) $(SYSCALL_C_OBJ) \
       $(VERSION_OBJ) $(FAT16_OBJ) $(ATA_OBJ) $(PROGRAM_OBJ) $(EDITOR_OBJ) $(ISO_FS_OBJ) $(ISO_FS_TEST_OBJ) \
       $(ANSI_OBJ) $(PROGRESS_OBJ)

# Add after your other file definitions
VBE_C = $(DRIVERS_DIR)/vbe.c
VBE_OBJ = $(BUILD_DIR)/vbe.o
FONT_C = $(DRIVERS_DIR)/font_8x16.c
FONT_OBJ = $(BUILD_DIR)/font_8x16.o

# Add ISO_FS_OBJ and ISO_FS_TEST_OBJ to the OBJS list
OBJS = $(BOOT_OBJ) $(KERNEL_OBJ) $(VGA_OBJ) $(IO_OBJ) $(IDT_ASM_OBJ) $(IDT_C_OBJ) \
       $(GDT_C_OBJ) $(VGA_DRIVER_OBJ) $(KEYBOARD_DRIVER_OBJ) $(TIMER_DRIVER_OBJ) \
       $(STRING_OBJ) $(SHELL_OBJ) $(PMM_OBJ) $(MEMORY_MAP_OBJ) $(HEAP_OBJ) $(STDLIB_OBJ) $(LIBGCC_OBJ) \
       $(TEST_OBJ) $(TEST2_OBJ) $(SYSCALL_TEST_OBJ) $(SYSCALL_ASM_OBJ) $(SYSCALL_C_OBJ) \
       $(VERSION_OBJ) $(FAT16_OBJ) $(ATA_OBJ) $(PROGRAM_OBJ) $(EDITOR_OBJ) $(ISO_FS_OBJ) $(ISO_FS_TEST_OBJ) \
       $(ANSI_OBJ) $(PROGRESS_OBJ) $(VBE_OBJ) $(FONT_OBJ)

# Default target
.PHONY: all
all: $(ISO_IMAGE) run clean

# Create build directories
$(BUILD_DIR):
	mkdir -p $@
	mkdir -p $(ISO_BOOT_DIR)
	mkdir -p $(ISO_GRUB_DIR)

$(BUILD_DIR)/tests:
	mkdir -p $@

# Assemble boot code
$(BOOT_OBJ): $(BOOT_ASM) | $(BUILD_DIR)
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

# Compile IDT assembly
$(IDT_ASM_OBJ): $(IDT_ASM) | $(BUILD_DIR)
	@echo "Assembling IDT..."
	$(ASM) $(ASMFLAGS) $< -o $@

# Compile IDT C
$(IDT_C_OBJ): $(IDT_C) | $(BUILD_DIR)
	@echo "Compiling IDT C..."
	$(CC) $(CFLAGS) $< -o $@

# Compile GDT C
$(GDT_C_OBJ): $(GDT_C) | $(BUILD_DIR)
	@echo "Compiling GDT C..."
	$(CC) $(CFLAGS) $< -o $@

# Compile VGA driver
$(VGA_DRIVER_OBJ): $(VGA_DRIVER_C) | $(BUILD_DIR)
	@echo "Compiling VGA driver..."
	$(CC) $(CFLAGS) $< -o $@

# Compile keyboard driver
$(KEYBOARD_DRIVER_OBJ): $(KEYBOARD_DRIVER_C) | $(BUILD_DIR)
	@echo "Compiling keyboard driver..."
	$(CC) $(CFLAGS) $< -o $@

# Compile timer driver
$(TIMER_DRIVER_OBJ): $(TIMER_DRIVER_C) | $(BUILD_DIR)
	@echo "Compiling timer driver..."
	$(CC) $(CFLAGS) $< -o $@

# Compile string functions
$(STRING_OBJ): $(STRING_C) | $(BUILD_DIR)
	@echo "Compiling string functions..."
	$(CC) $(CFLAGS) $< -o $@

# Compile shell
$(SHELL_OBJ): $(SHELL_C) | $(BUILD_DIR)
	@echo "Compiling shell..."
	$(CC) $(CFLAGS) $< -o $@

# Compile PMM
$(PMM_OBJ): $(PMM_C) | $(BUILD_DIR)
	@echo "Compiling PMM..."
	$(CC) $(CFLAGS) $< -o $@

# Compile Memory Map
$(MEMORY_MAP_OBJ): $(MEMORY_MAP_C) | $(BUILD_DIR)
	@echo "Compiling Memory Map..."
	$(CC) $(CFLAGS) $< -o $@

# Compile heap
$(HEAP_OBJ): $(HEAP_C) | $(BUILD_DIR)
	@echo "Compiling heap..."
	$(CC) $(CFLAGS) $< -o $@

# Compile stdlib
$(STDLIB_OBJ): $(STDLIB_C) | $(BUILD_DIR)
	@echo "Compiling stdlib..."
	$(CC) $(CFLAGS) $< -o $@

# Compile libgcc
$(LIBGCC_OBJ): $(LIBGCC_C) | $(BUILD_DIR)
	@echo "Compiling libgcc..."
	$(CC) $(CFLAGS) $< -o $@

# Compile test
$(TEST_OBJ): $(TEST_C) | $(BUILD_DIR)/tests
	@echo "Compiling Memory Test..."
	$(CC) $(CFLAGS) $< -o $@

# Compile syscall assembly
$(SYSCALL_ASM_OBJ): $(SYSCALL_ASM) | $(BUILD_DIR)
	@echo "Assembling syscall..."
	$(ASM) $(ASMFLAGS) $< -o $@

# Compile syscall C
$(SYSCALL_C_OBJ): $(SYSCALL_C) | $(BUILD_DIR)
	@echo "Compiling syscall C..."
	$(CC) $(CFLAGS) $< -o $@

# Compile syscall test
$(SYSCALL_TEST_OBJ): $(SYSCALL_TEST_C) | $(BUILD_DIR)
	@echo "Compiling syscall test..."
	$(CC) $(CFLAGS) $< -o $@

# Compile ANSI support
$(ANSI_OBJ): $(ANSI_C) | $(BUILD_DIR)
	@echo "Compiling ANSI support..."
	$(CC) $(CFLAGS) $< -o $@

# Compile version
$(VERSION_OBJ): $(VERSION_C) | $(BUILD_DIR)
	@echo "Compiling version..."
	$(CC) $(CFLAGS) $< -o $@

# Compile FAT16 filesystem
$(FAT16_OBJ): $(FAT16_C) | $(BUILD_DIR)
	@echo "Compiling FAT16 filesystem..."
	$(CC) $(CFLAGS) $< -o $@

# Compile ATA driver
$(ATA_OBJ): $(ATA_C) | $(BUILD_DIR)
	@echo "Compiling ATA driver..."
	$(CC) $(CFLAGS) $< -o $@

# Compile program
$(PROGRAM_OBJ): $(PROGRAM_C) | $(BUILD_DIR)
	@echo "Compiling program..."
	$(CC) $(CFLAGS) $< -o $@

# Compile test2 (src/test.c)
$(TEST2_OBJ): $(TEST2_C) | $(BUILD_DIR)
	@echo "Compiling test2 (memory management test)..."
	$(CC) $(CFLAGS) $< -o $@

# Compile editor
$(EDITOR_OBJ): $(EDITOR_C) | $(BUILD_DIR)
	@echo "Compiling editor..."
	$(CC) $(CFLAGS) $< -o $@

# Compile ISO filesystem driver
$(ISO_FS_OBJ): $(ISO_FS_C) | $(BUILD_DIR)
	@echo "Compiling ISO filesystem driver..."
	$(CC) $(CFLAGS) $< -o $@

# Compile ISO filesystem test
$(ISO_FS_TEST_OBJ): $(ISO_FS_TEST_C) | $(BUILD_DIR)/tests
	@echo "Compiling ISO filesystem test..."
	$(CC) $(CFLAGS) $< -o $@

# Compile progress bar
$(PROGRESS_OBJ): $(PROGRESS_C) | $(BUILD_DIR)
	@echo "Compiling progress bar..."
	$(CC) $(CFLAGS) $< -o $@

# Compile VBE driver
$(VBE_OBJ): $(VBE_C) | $(BUILD_DIR)
	@echo "Compiling VBE driver..."
	$(CC) $(CFLAGS) $< -o $@

# Compile font
$(FONT_OBJ): $(FONT_C) | $(BUILD_DIR)
	@echo "Compiling font..."
	$(CC) $(CFLAGS) $< -o $@

# Link kernel
$(KERNEL_BIN): $(OBJS)
	@echo "Linking kernel..."
	$(LD) $(LDFLAGS) $^ -o $@

# Create ISO
$(ISO_IMAGE): $(KERNEL_BIN) | $(BUILD_DIR)
	@echo "Creating ISO..."
	cp $(KERNEL_BIN) $(ISO_BOOT_DIR)/kernel.bin
	cp grub.cfg $(ISO_GRUB_DIR)/grub.cfg
	cp fat16.img $(ISO_DIR)/fat16.img
	grub-mkrescue -o $@ $(ISO_DIR)

# Run in QEMU
.PHONY: run
run: $(ISO_IMAGE)
	@echo "Running in QEMU..."
	qemu-system-i386 -machine pc -m 2G -cdrom $(ISO_IMAGE) -hda fat16.img -boot d

# Clean build files
.PHONY: clean
clean:
	@echo "Cleaning..."
	rm -rf $(BUILD_DIR)

# Help target
.PHONY: help
help:
	@echo "Available targets:"
	@echo "  all     - Build the OS and create ISO image"
	@echo "  run     - Run the OS in QEMU"
	@echo "  clean   - Remove all build files"
	@echo "  help    - Show this help message"
