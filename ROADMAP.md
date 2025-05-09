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
- [~] Design simple file system
  - [x] Basic directory structure (FAT16)
  - [~] File operations
    - [x] Directory listing (ls)
    - [x] File reading (cat)
    - [ ] File writing
    - [ ] File creation
    - [ ] File deletion
- [~] Implement FAT16 support
  - [x] File system driver initialization
  - [x] Root directory listing
  - [ ] Directory traversal
  - [ ] File reading implementation
  - [ ] File writing implementation
  - [ ] File system integrity checks
  - [ ] Error handling and recovery

## Phase 4: Shell and Basic Commands
- [x] Implement command-line shell
  - [x] Command parser
  - [ ] Command history
  - [x] Basic command execution
- [x] Create basic commands
  - [x] `ls` - List root directory contents (read-only)
  - [ ] `cd` - Change directory
  - [x] `cat` - Display file contents
  - [x] `echo` - Print text
  - [x] `help` - Show available commands
  - [x] `clear` - Clear the screen
  - [x] `disktest` - Run disk driver tests
- [ ] Implement TSR (Terminate and Stay Resident) support
  - [ ] Basic program loading
  - [ ] Memory management for TSR programs
  - [ ] Program termination handling

## Phase 5: Memory Management
- [x] Implement basic memory management
  - [x] Memory allocation/deallocation
  - [x] Memory map handling
  - [x] Memory testing
  - [ ] Memory protection
  - [ ] Program loading and unloading
  - [ ] TSR program memory management

## Phase 6: Device Drivers
- [x] Implement basic device drivers
  - [x] VGA driver
  - [x] Keyboard driver registration
  - [ ] Serial port driver
  - [ ] Timer driver
- [~] Add support for additional devices
  - [x] Hard disk driver (read-only, for FAT16 ls)

## Phase 7: User Interface Enhancements
- [x] Improve terminal interface
  - [x] Command line editing
  - [x] Color support
  - [x] Cursor movement
  - [x] Debug output system
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
- Bootloader, kernel, VGA, keyboard, and memory management are implemented and stable
- Shell is functional with command parsing and basic commands (`ls`, `echo`, `help`, `clear`, `disktest`)
- FAT16 read-only root directory listing works via `ls` command
- Disk driver works for reading sectors from a QEMU-attached FAT16 image
- Modular codebase with clear separation of drivers, shell, and file system
- Keyboard driver now supports buffered input and proper key handling
- Terminal interface supports command line editing and cursor movement

## Next Steps
1. Implement file reading (`cat` command) from FAT16
2. Add support for changing directories (`cd`)
3. Add file write support (optional, for full FAT16 support)
4. Implement command history and tab completion in the shell
5. Add timer and serial port drivers
6. Expand documentation and add more tests

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