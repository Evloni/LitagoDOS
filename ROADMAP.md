# LitagoDOS - Terminal OS Development Roadmap

## Phase 1: Bootloader and Basic Setup using multiboot
- [x] Set up development environment
  - [x] Install necessary tools (NASM, GCC, QEMU)
  - [x] Configure build system
- [x] Create basic bootloader
  - [x] Implement BIOS boot sector
  - [x] Set up memory segments
  - [x] Switch to 32-bit protected mode
- [x] Implement basic kernel entry point
  - [x] Set up stack
  - [x] Initialize basic hardware

## Phase 2: Kernel Core
- [ ] Implement basic kernel functions
  - [ ] Memory management
  - [ ] Interrupt handling
  - [ ] Basic I/O operations
    - [x] Basic output (VGA text mode)
    - [ ] Basic input (keyboard)
- [x] Set up terminal interface
  - [x] VGA text mode support
  - [x] Basic text output
  - [x] Cursor support
  - [ ] Keyboard input handling
- [ ] Implement basic system calls

## Phase 3: File System
- [ ] Design simple file system
  - [ ] Basic directory structure
  - [ ] File operations (create, read, write, delete)
- [ ] Implement FAT12/16 support
  - [ ] File system driver
  - [ ] Basic file operations

## Phase 4: Shell and Basic Commands
- [ ] Implement command-line shell
  - [ ] Command parser
  - [ ] Command history
  - [ ] Basic command execution
- [ ] Create basic commands
  - [ ] `ls` - List directory contents
  - [ ] `cd` - Change directory
  - [ ] `cat` - Display file contents
  - [ ] `echo` - Print text
  - [ ] `help` - Show available commands
- [ ] Implement TSR (Terminate and Stay Resident) support
  - [ ] Basic program loading
  - [ ] Memory management for TSR programs
  - [ ] Program termination handling

## Phase 5: Memory Management
- [ ] Implement memory management
  - [ ] Memory allocation/deallocation
  - [ ] Memory protection
  - [ ] Program loading and unloading
  - [ ] TSR program memory management

## Phase 6: Device Drivers
- [ ] Implement basic device drivers
  - [ ] Keyboard driver
  - [ ] Serial port driver
  - [ ] Timer driver
- [ ] Add support for additional devices
  - [ ] Floppy disk driver
  - [ ] Hard disk driver

## Phase 7: User Interface Enhancements
- [x] Improve terminal interface
  - [x] Command line editing
  - [x] Color support
  - [x] Cursor movement
  - [ ] Tab completion
- [ ] Add basic text editor
  - [ ] Line editing
  - [ ] File saving/loading

## Phase 8: Networking (Optional)
- [ ] Implement basic networking
  - [ ] Network driver
  - [ ] TCP/IP stack
  - [ ] Basic network commands

## Phase 9: Testing and Documentation
- [ ] Create comprehensive test suite
  - [ ] Unit tests
  - [ ] Integration tests
  - [ ] System tests
- [ ] Write documentation
  - [ ] User manual
  - [ ] Developer guide
  - [ ] API documentation

## Current Progress
- Successfully implemented GRUB multiboot bootloader
- Set up basic kernel structure with modular design
- Implemented VGA text mode driver with:
  - Text output
  - Color support
  - Cursor control
  - Newline handling
- Organized code into separate modules (kernel, VGA)

## Next Steps
1. Implement keyboard input handling
2. Add interrupt handling
3. Implement basic memory management
4. Create simple command parser
5. Add basic file system support

## Development Guidelines
1. Start with the simplest working version and iterate
2. Test each component thoroughly before moving to the next phase
3. Keep the code modular and well-documented
4. Use version control (Git) from the beginning
5. Regular backups of development environment

## Tools and Technologies
- Assembly (NASM) for bootloader and low-level code
- C for kernel and higher-level components
- QEMU for testing and debugging
- Git for version control
- Make for build automation

## Getting Started
1. Set up development environment
2. Create basic bootloader
3. Implement kernel entry point
4. Add basic terminal output
5. Implement keyboard input
6. Create basic shell

## Resources
- OSDev Wiki (https://wiki.osdev.org)
- Intel Software Developer Manuals
- x86 Assembly Language Reference
- Operating System Design Books 