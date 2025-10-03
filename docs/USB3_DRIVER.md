# USB 3.0 (xHCI) Driver Documentation

## Overview

This USB 3.0 driver implements support for the **eXtensible Host Controller Interface (xHCI)** specification, which is the standard for USB 3.0 host controllers. The driver can detect and initialize xHCI controllers, enumerate ports, and manage USB devices.

## Architecture

### Components

1. **xHCI Controller Detection** - Scans PCI bus for xHCI-compatible controllers
2. **Controller Initialization** - Resets and starts the xHCI controller
3. **Ring Management** - Manages command and event rings for communication
4. **Port Management** - Detects and manages USB ports and connected devices
5. **Device Enumeration** - Identifies connected USB devices (future work)

### Key Structures

#### `xhci_controller_t`
Main controller structure containing:
- PCI information (bus, device, function)
- Memory-mapped register bases
- Controller capabilities (max slots, ports, interrupters)
- Command and event rings
- Device Context Base Address Array (DCBAA)
- Connected device information

#### `xhci_trb_t` (Transfer Request Block)
Basic communication unit for xHCI:
- 64-bit parameter field
- 32-bit status field
- 32-bit control field with TRB type and cycle bit

#### `xhci_device_t`
Structure representing a USB device:
- Slot ID and port number
- Device speed (Low, Full, High, SuperSpeed)
- Device and input contexts
- Connection status

## Memory-Mapped Register Layout

### Capability Registers (Base + 0x00)
- **CAPLENGTH** (0x00): Operational registers offset
- **HCIVERSION** (0x02): xHCI version
- **HCSPARAMS1-3** (0x04-0x0C): Structural parameters
- **HCCPARAMS1** (0x10): Capability parameters
- **DBOFF** (0x14): Doorbell array offset
- **RTSOFF** (0x18): Runtime registers offset

### Operational Registers (Base + CAPLENGTH)
- **USBCMD** (0x00): Command register (Run/Stop, Reset, Interrupts)
- **USBSTS** (0x04): Status register (Halted, Ready, Errors)
- **PAGESIZE** (0x08): Page size
- **CRCR** (0x18): Command Ring Control Register
- **DCBAAP** (0x30): Device Context Base Address Array Pointer
- **CONFIG** (0x38): Configuration register

### Port Registers (Base + CAPLENGTH + 0x400)
Each port has 4 32-bit registers (16 bytes apart):
- **PORTSC** (0x00): Port Status and Control
  - CCS (bit 0): Current Connect Status
  - PED (bit 1): Port Enabled/Disabled
  - PR (bit 4): Port Reset
  - Port Speed (bits 10-13)
  - Various change bits (CSC, PEC, etc.)

## USB Speeds Supported

| Speed | Value | Description | Bandwidth |
|-------|-------|-------------|-----------|
| Full-Speed | 1 | USB 1.1 | 12 Mbps |
| Low-Speed | 2 | USB 1.0 | 1.5 Mbps |
| High-Speed | 3 | USB 2.0 | 480 Mbps |
| SuperSpeed | 4 | USB 3.0 | 5 Gbps |

## Initialization Process

### 1. Controller Detection
```
xhci_scan_devices()
  └── Scan PCI bus (0-255, devices 0-31, functions 0-7)
      └── Check class code: 0x0C (Serial Bus) / 0x03 (USB) / 0x30 (xHCI)
          └── xhci_detect_controller(bus, device, function)
```

### 2. Controller Initialization
```
xhci_detect_controller()
  ├── Read PCI BAR0 for memory-mapped base address
  ├── Parse capability registers
  ├── Calculate operational/runtime/doorbell register addresses
  ├── xhci_reset_controller()
  │   ├── Stop controller (clear Run bit)
  │   ├── Wait for Halted status
  │   ├── Set Reset bit
  │   └── Wait for reset complete and controller ready
  ├── xhci_start_controller()
  │   ├── Allocate Device Context Base Address Array (DCBAA)
  │   ├── Configure max device slots
  │   ├── Initialize command ring
  │   ├── Initialize event ring (with ERST)
  │   ├── Enable interrupts
  │   └── Set Run bit
  └── xhci_probe_ports()
      └── Check each port for connected devices
```

### 3. Ring Initialization

#### Command Ring
- 256 TRB circular buffer
- Producer (driver) posts commands
- Consumer (controller) processes them
- Cycle bit tracks ring wrap-around

#### Event Ring
- 256 TRB circular buffer
- Producer (controller) posts events
- Consumer (driver) processes them
- Uses Event Ring Segment Table (ERST)

## Usage

### Shell Command
```bash
usb
```

This command will:
1. Scan for xHCI controllers on the PCI bus
2. Initialize the first controller found
3. Display controller information:
   - PCI location (bus:device.function)
   - Base address
   - xHCI version
   - Maximum slots, ports, and interrupters
4. Probe all ports and show connected devices

### Sample Output
```
=== USB 3.0 (xHCI) Driver Initialization ===

Scanning for USB 3.0 (xHCI) controllers...

Found xHCI controller at 0:4.0
xHCI Base Address: 0xfeb00000
xHCI Version: 1.0
Max Device Slots: 15
Max Interrupters: 8
Max Ports: 4
Resetting xHCI controller...
xHCI controller reset complete
Starting xHCI controller...
Command ring initialized at 0x02000000
Event ring initialized at 0x02001000
xHCI controller started successfully

Probing USB ports...
Number of ports: 4

Port 1: Device connected - Enabled - SuperSpeed (5 Gbps)
Port 2: No device
Port 3: Device connected - Enabled - High-Speed (480 Mbps)
Port 4: No device

USB 3.0 driver initialized successfully
```

## QEMU Testing

The Makefile includes xHCI support for QEMU:
```bash
qemu-system-i386 -machine q35 -m 2G -cdrom LitagoOS.iso -usb -device qemu-xhci
```

Options:
- `-usb`: Enable USB support
- `-device qemu-xhci`: Add xHCI (USB 3.0) controller
- You can add USB devices with: `-device usb-kbd`, `-device usb-mouse`, `-device usb-storage,drive=...`

## Current Limitations

1. **Device Enumeration**: Basic port detection only - full device enumeration is not yet implemented
2. **Data Transfer**: No transfer implementation yet (bulk, control, interrupt, isochronous)
3. **USB Descriptors**: Device, configuration, and endpoint descriptor parsing not implemented
4. **Class Drivers**: No specific drivers for USB classes (HID, mass storage, etc.)
5. **Hot-plug**: Device attach/detach events not fully handled
6. **Multiple Controllers**: Only the first xHCI controller is initialized

## Future Enhancements

### Phase 1: Basic Device Enumeration
- [ ] Implement Enable Slot command
- [ ] Implement Address Device command
- [ ] Read device descriptors
- [ ] Configure endpoint contexts

### Phase 2: Control Transfers
- [ ] Implement control transfer mechanism
- [ ] Setup stage (SETUP TRB)
- [ ] Data stage (DATA TRB)
- [ ] Status stage (STATUS TRB)

### Phase 3: Bulk Transfers
- [ ] Implement bulk transfer mechanism
- [ ] Transfer ring management per endpoint
- [ ] Short packet handling

### Phase 4: Class Drivers
- [ ] USB HID (Human Interface Device) driver
  - Keyboard support
  - Mouse support
- [ ] USB Mass Storage driver
  - Bulk-Only Transport (BOT)
  - SCSI command set
- [ ] USB Hub driver

### Phase 5: Advanced Features
- [ ] Interrupt transfers (for periodic data)
- [ ] Isochronous transfers (for audio/video)
- [ ] USB 3.0 streams
- [ ] USB power management
- [ ] Multiple xHCI controller support

## Technical References

1. **xHCI Specification**: Intel eXtensible Host Controller Interface for USB (xHCI) Specification, Revision 1.2
2. **USB 3.0 Specification**: Universal Serial Bus 3.0 Specification
3. **PCI Configuration Space**: PCI Local Bus Specification

## File Structure

```
include/drivers/xhci.h    - Header file with structures and definitions
src/drivers/xhci.c        - Main xHCI driver implementation
src/shell/shell.c         - Shell integration ('usb' command)
Makefile                  - Build system integration
```

## API Reference

### Main Functions

```c
bool xhci_init(void)
```
Initialize the USB 3.0 subsystem. Scans for controllers and initializes the first one found.

```c
void xhci_scan_devices(void)
```
Scan the PCI bus for xHCI controllers.

```c
bool xhci_detect_controller(uint8_t bus, uint8_t device, uint8_t function)
```
Detect and initialize a specific xHCI controller.

### Controller Management

```c
bool xhci_reset_controller(xhci_controller_t* xhci)
```
Reset an xHCI controller to a known state.

```c
bool xhci_start_controller(xhci_controller_t* xhci)
```
Start a previously reset xHCI controller.

### Port Management

```c
void xhci_probe_ports(xhci_controller_t* xhci)
```
Probe all ports for connected devices.

```c
uint32_t xhci_read_port_reg(xhci_controller_t* xhci, uint8_t port, uint8_t reg)
```
Read a port register.

```c
void xhci_write_port_reg(xhci_controller_t* xhci, uint8_t port, uint8_t reg, uint32_t value)
```
Write to a port register.

```c
void xhci_reset_port(xhci_controller_t* xhci, uint8_t port)
```
Reset a specific USB port.

### Ring Management

```c
bool xhci_init_command_ring(xhci_controller_t* xhci)
```
Initialize the command ring for sending commands to the controller.

```c
bool xhci_init_event_ring(xhci_controller_t* xhci)
```
Initialize the event ring for receiving events from the controller.

```c
bool xhci_post_command(xhci_controller_t* xhci, xhci_trb_t* trb)
```
Post a command TRB to the command ring.

```c
bool xhci_process_events(xhci_controller_t* xhci)
```
Process pending events from the event ring.

## Debugging Tips

1. **Enable QEMU Logging**: Add `-d int,cpu_reset` to see interrupts and resets
2. **Check PCI Configuration**: Use the `pci` shell command to verify the controller is detected
3. **Monitor Register Values**: Add printf statements to track register reads/writes
4. **Verify Memory Addresses**: Ensure ring buffers are properly aligned (64-byte for device contexts, 16-byte for TRBs)
5. **Check Timeouts**: If initialization hangs, increase timeout values or check for stuck bits

## Contributing

When extending this driver:
1. Follow the xHCI specification carefully
2. Add comprehensive error checking
3. Document register access and TRB structures
4. Test with multiple USB device types
5. Update this documentation with new features

## License

Part of LitagoDOS - see main project license.

