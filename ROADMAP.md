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
  - [x] Configure multiboot header correctly

## Phase 2: Kernel Core
- [x] Implement basic kernel functions
  - [x] Memory management
    - [x] Physical memory manager (PMM)
    - [x] Memory map initialization
    - [x] Memory allocation/deallocation
    - [x] Memory testing
  - [x] Interrupt handling
    - [x] GDT initialization
    - [x] IDT initialization
  - [x] Basic I/O operations
    - [x] Basic output (VGA text mode)
    - [x] Basic input (keyboard driver registration)
- [x] Set up terminal interface
  - [x] VGA text mode support
  - [x] Basic text output
  - [x] Cursor support
  - [x] Keyboard input handling
  - [x] Debug output system
- [x] Implement basic system calls

## Phase 3: File System
- [x] Design simple file system
  - [x] Basic directory structure
  - [x] File operations (create, read, write, delete)
- [x] Implement FAT16 support
  - [x] File system driver
  - [x] File read support (cat)
  - [x] File write support
  - [x] File deletion support (rm)
  - [x] Directory listing (ls)

## Phase 4: Shell and Basic Commands
- [x] Implement command-line shell
  - [x] Command parser
  - [x] Command history with arrow key navigation
  - [x] Basic command execution
- [x] Create basic commands
  - [x] `ls` - List directory contents
  - [x] `cd` - Change directory
  - [x] `cat` - Display file contents
  - [x] `echo` - Print text
  - [x] `help` - Show available commands
  - [x] `clear` - Clear the terminal
  - [x] `disktest` - Test disk driver
  - [x] `memtest` - Basic memory test
  - [x] `memtest2` - Advanced memory management test (PMM + heap)
  - [x] `edit` - Text editor
  - [x] `rm` - Remove files

## Phase 5: Memory Management
- [x] Implement basic memory management
  - [x] Memory allocation/deallocation
  - [x] Memory map handling
  - [x] Memory testing
  - [x] Memory protection (removed for DOS-like mode)
  - [x] Program loading and unloading
  - [x] Advanced memory management test (shell command `memtest2`)

## Phase 6: Device Drivers
- [x] Implement basic device drivers
  - [x] VGA driver
  - [x] Keyboard driver with arrow key support
  - [x] Disk driver (QEMU-attached FAT16 image)
  - [ ] Serial port driver
  - [x] Timer driver
- [ ] Add support for additional devices
  - [ ] Floppy disk driver
  - [ ] Hard disk driver

## Phase 7: User Interface Enhancements
- [x] Improve terminal interface
  - [x] Command line editing
  - [x] Color support
  - [x] Cursor movement
  - [x] Debug output system
  - [ ] Tab completion
- [x] Add basic text editor
  - [x] Line editing
  - [x] File saving/loading
  - [x] Cursor movement
  - [x] Status bar

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
- Bootloader, kernel, VGA, keyboard, and memory management are implemented and stable
- Shell is functional with command parsing, command history navigation, and basic commands
- FAT16 file system fully implemented with read/write/delete support
- Text editor implemented with cursor movement and file operations
- Modular codebase with clear separation of drivers, shell, and file system
- Memory protection and paging removed for DOS-like authenticity
- Advanced memory management test available via `memtest2` shell command

## Next Steps
1. Implement tab completion in shell
2. Add syntax highlighting to text editor
3. Implement serial port driver
4. Add floppy and hard disk drivers
5. Create comprehensive test suite

## Development Guidelines
1. Start with the simplest working version and iterate
2. Test each component thoroughly before moving to the next phase
3. Keep the code modular and well-documented
4. Use version control (Git) from the beginning
5. Regular backups of development environment
6. Maintain clean and minimal output for production builds

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