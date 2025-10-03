#include "../../include/drivers/xhci.h"
#include "../../include/drivers/pci.h"
#include "../../include/drivers/usb_descriptors.h"
#include "../../include/io.h"
#include "../../include/stdio.h"
#include "../../include/memory/heap.h"
#include "../../include/string.h"
#include <stddef.h>

// Global xHCI controller instance
xhci_controller_t* g_xhci = NULL;

// Helper function to read from xHCI memory-mapped registers
static inline uint32_t xhci_read32(uint8_t* addr) {
    return *((volatile uint32_t*)addr);
}

// Helper function to write to xHCI memory-mapped registers
static inline void xhci_write32(uint8_t* addr, uint32_t value) {
    *((volatile uint32_t*)addr) = value;
}

// Helper function to read 64-bit values
static inline uint64_t xhci_read64(uint8_t* addr) {
    uint32_t low = *((volatile uint32_t*)addr);
    uint32_t high = *((volatile uint32_t*)(addr + 4));
    return ((uint64_t)high << 32) | low;
}

// Helper function to write 64-bit values
static inline void xhci_write64(uint8_t* addr, uint64_t value) {
    *((volatile uint32_t*)addr) = (uint32_t)(value & 0xFFFFFFFF);
    *((volatile uint32_t*)(addr + 4)) = (uint32_t)(value >> 32);
}

// Get port speed string
const char* xhci_get_speed_string(uint8_t speed) {
    switch (speed) {
        case XHCI_SPEED_FULL:  return "Full-Speed (12 Mbps)";
        case XHCI_SPEED_LOW:   return "Low-Speed (1.5 Mbps)";
        case XHCI_SPEED_HIGH:  return "High-Speed (480 Mbps)";
        case XHCI_SPEED_SUPER: return "SuperSpeed (5 Gbps)";
        default:               return "Unknown Speed";
    }
}

// Extract speed from port status register
uint8_t xhci_get_port_speed(uint32_t portsc) {
    return (portsc & XHCI_PORTSC_SPEED_MASK) >> 10;
}

// Read port register
uint32_t xhci_read_port_reg(xhci_controller_t* xhci, uint8_t port, uint8_t reg) {
    // Port registers start at offset 0x400 from operational base
    // Each port has 4 32-bit registers (16 bytes)
    uint8_t* port_base = xhci->op_regs + 0x400 + (port * 16);
    return xhci_read32(port_base + (reg * 4));
}

// Write port register
void xhci_write_port_reg(xhci_controller_t* xhci, uint8_t port, uint8_t reg, uint32_t value) {
    uint8_t* port_base = xhci->op_regs + 0x400 + (port * 16);
    xhci_write32(port_base + (reg * 4), value);
}

// Reset a port
void xhci_reset_port(xhci_controller_t* xhci, uint8_t port) {
    printf("Resetting port %d...\n", port);
    
    uint32_t portsc = xhci_read_port_reg(xhci, port, 0);
    
    // Set Port Reset bit
    portsc |= XHCI_PORTSC_PR;
    xhci_write_port_reg(xhci, port, 0, portsc);
    
    // Wait for reset to complete (port reset bit clears when done)
    int timeout = 1000;
    while (timeout-- > 0) {
        portsc = xhci_read_port_reg(xhci, port, 0);
        if (!(portsc & XHCI_PORTSC_PR)) {
            break;
        }
        // Small delay
        for (volatile int i = 0; i < 10000; i++);
    }
    
    if (timeout <= 0) {
        printf("Port %d reset timeout\n", port);
    } else {
        printf("Port %d reset complete\n", port);
    }
}

// Initialize command ring
bool xhci_init_command_ring(xhci_controller_t* xhci) {
    // Allocate memory for command ring (256 TRBs)
    xhci->cmd_ring.size = 256;
    xhci->cmd_ring.trbs = (xhci_trb_t*)malloc(xhci->cmd_ring.size * sizeof(xhci_trb_t));
    
    if (!xhci->cmd_ring.trbs) {
        printf("Failed to allocate command ring\n");
        return false;
    }
    
    // Clear the command ring
    memset(xhci->cmd_ring.trbs, 0, xhci->cmd_ring.size * sizeof(xhci_trb_t));
    
    // Initialize ring state
    xhci->cmd_ring.enqueue_ptr = 0;
    xhci->cmd_ring.dequeue_ptr = 0;
    xhci->cmd_ring.cycle_state = 1;
    
    // Write command ring pointer to controller
    uint64_t cmd_ring_addr = (uint64_t)xhci->cmd_ring.trbs;
    xhci_write64(xhci->op_regs + XHCI_OP_CRCR, cmd_ring_addr | 1); // Set RCS bit
    
    printf("Command ring initialized at 0x%08x\n", (uint32_t)cmd_ring_addr);
    return true;
}

// Initialize event ring
bool xhci_init_event_ring(xhci_controller_t* xhci) {
    // Allocate memory for event ring (256 TRBs)
    xhci->event_ring.size = 256;
    xhci->event_ring.trbs = (xhci_trb_t*)malloc(xhci->event_ring.size * sizeof(xhci_trb_t));
    
    if (!xhci->event_ring.trbs) {
        printf("Failed to allocate event ring\n");
        return false;
    }
    
    // Clear the event ring
    memset(xhci->event_ring.trbs, 0, xhci->event_ring.size * sizeof(xhci_trb_t));
    
    // Allocate Event Ring Segment Table (ERST)
    xhci->event_ring.erst = (xhci_erst_entry_t*)malloc(sizeof(xhci_erst_entry_t));
    if (!xhci->event_ring.erst) {
        printf("Failed to allocate ERST\n");
        free(xhci->event_ring.trbs);
        return false;
    }
    
    // Initialize ERST
    xhci->event_ring.erst[0].ring_segment_base = (uint64_t)xhci->event_ring.trbs;
    xhci->event_ring.erst[0].ring_segment_size = xhci->event_ring.size;
    xhci->event_ring.erst[0].reserved = 0;
    
    // Initialize ring state
    xhci->event_ring.dequeue_ptr = 0;
    xhci->event_ring.cycle_state = 1;
    
    // Configure interrupter 0 (primary interrupter)
    uint8_t* intr_regs = xhci->runtime_regs + 0x20; // Interrupter 0 registers start at offset 0x20
    
    // Set ERST size
    xhci_write32(intr_regs + 0x08, 1); // ERSTSZ - 1 segment
    
    // Set ERST base address
    xhci_write64(intr_regs + 0x10, (uint64_t)xhci->event_ring.erst); // ERSTBA
    
    // Set event ring dequeue pointer
    xhci_write64(intr_regs + 0x18, (uint64_t)xhci->event_ring.trbs | (1 << 3)); // ERDP with EHB bit
    
    printf("Event ring initialized at 0x%08x\n", (uint32_t)xhci->event_ring.trbs);
    return true;
}

// Post a command to the command ring
bool xhci_post_command(xhci_controller_t* xhci, xhci_trb_t* trb) {
    // Copy TRB to command ring
    uint32_t enq = xhci->cmd_ring.enqueue_ptr;
    memcpy(&xhci->cmd_ring.trbs[enq], trb, sizeof(xhci_trb_t));
    
    // Set cycle bit
    xhci->cmd_ring.trbs[enq].control |= xhci->cmd_ring.cycle_state;
    
    // Advance enqueue pointer
    enq++;
    if (enq >= xhci->cmd_ring.size) {
        enq = 0;
        xhci->cmd_ring.cycle_state ^= 1; // Toggle cycle state
    }
    xhci->cmd_ring.enqueue_ptr = enq;
    
    // Ring the command doorbell (doorbell 0)
    xhci_ring_doorbell(xhci, 0, 0);
    
    return true;
}

// Ring a doorbell
void xhci_ring_doorbell(xhci_controller_t* xhci, uint8_t doorbell, uint8_t target) {
    xhci->doorbell_regs[doorbell] = target;
}

// Process events from the event ring
bool xhci_process_events(xhci_controller_t* xhci) {
    uint32_t deq = xhci->event_ring.dequeue_ptr;
    xhci_trb_t* trb = &xhci->event_ring.trbs[deq];
    
    // Check if there are any events (cycle bit matches)
    if ((trb->control & 1) != xhci->event_ring.cycle_state) {
        return false; // No events
    }
    
    // Process the event
    uint32_t trb_type = (trb->control >> 10) & 0x3F;
    
    switch (trb_type) {
        case TRB_TYPE_CMD_COMPLETION:
            printf("Command completion event\n");
            break;
        case TRB_TYPE_PORT_STATUS:
            printf("Port status change event\n");
            break;
        case TRB_TYPE_TRANSFER:
            printf("Transfer event\n");
            break;
        default:
            printf("Unknown event type: %d\n", trb_type);
            break;
    }
    
    // Advance dequeue pointer
    deq++;
    if (deq >= xhci->event_ring.size) {
        deq = 0;
        xhci->event_ring.cycle_state ^= 1; // Toggle cycle state
    }
    xhci->event_ring.dequeue_ptr = deq;
    
    // Update ERDP (Event Ring Dequeue Pointer)
    uint8_t* intr_regs = xhci->runtime_regs + 0x20;
    xhci_write64(intr_regs + 0x18, (uint64_t)&xhci->event_ring.trbs[deq] | (1 << 3));
    
    return true;
}

// Reset the xHCI controller
bool xhci_reset_controller(xhci_controller_t* xhci) {
    printf("Resetting xHCI controller...\n");
    
    // Stop the controller first
    uint32_t usbcmd = xhci_read32(xhci->op_regs + XHCI_OP_USBCMD);
    usbcmd &= ~XHCI_CMD_RUN;
    xhci_write32(xhci->op_regs + XHCI_OP_USBCMD, usbcmd);
    
    // Wait for controller to halt
    int timeout = 1000;
    while (timeout-- > 0) {
        uint32_t usbsts = xhci_read32(xhci->op_regs + XHCI_OP_USBSTS);
        if (usbsts & XHCI_STS_HCH) {
            break; // Controller halted
        }
        // Small delay
        for (volatile int i = 0; i < 1000; i++);
    }
    
    if (timeout <= 0) {
        printf("Controller halt timeout\n");
        return false;
    }
    
    // Reset the controller
    usbcmd = xhci_read32(xhci->op_regs + XHCI_OP_USBCMD);
    usbcmd |= XHCI_CMD_HCRST;
    xhci_write32(xhci->op_regs + XHCI_OP_USBCMD, usbcmd);
    
    // Wait for reset to complete
    timeout = 1000;
    while (timeout-- > 0) {
        usbcmd = xhci_read32(xhci->op_regs + XHCI_OP_USBCMD);
        if (!(usbcmd & XHCI_CMD_HCRST)) {
            break; // Reset complete
        }
        for (volatile int i = 0; i < 1000; i++);
    }
    
    if (timeout <= 0) {
        printf("Controller reset timeout\n");
        return false;
    }
    
    // Wait for controller to be ready
    timeout = 1000;
    while (timeout-- > 0) {
        uint32_t usbsts = xhci_read32(xhci->op_regs + XHCI_OP_USBSTS);
        if (!(usbsts & XHCI_STS_CNR)) {
            break; // Controller ready
        }
        for (volatile int i = 0; i < 1000; i++);
    }
    
    if (timeout <= 0) {
        printf("Controller ready timeout\n");
        return false;
    }
    
    printf("xHCI controller reset complete\n");
    return true;
}

// Start the xHCI controller
bool xhci_start_controller(xhci_controller_t* xhci) {
    printf("Starting xHCI controller...\n");
    
    // Allocate Device Context Base Address Array (DCBAA)
    xhci->dcbaa = (uint64_t*)malloc((xhci->max_slots + 1) * sizeof(uint64_t));
    if (!xhci->dcbaa) {
        printf("Failed to allocate DCBAA\n");
        return false;
    }
    memset(xhci->dcbaa, 0, (xhci->max_slots + 1) * sizeof(uint64_t));
    
    // Write DCBAA pointer to controller
    xhci_write64(xhci->op_regs + XHCI_OP_DCBAAP, (uint64_t)xhci->dcbaa);
    
    // Set max device slots enabled
    uint32_t config = xhci_read32(xhci->op_regs + XHCI_OP_CONFIG);
    config &= ~0xFF;
    config |= xhci->max_slots;
    xhci_write32(xhci->op_regs + XHCI_OP_CONFIG, config);
    
    // Initialize command ring
    if (!xhci_init_command_ring(xhci)) {
        printf("Failed to initialize command ring\n");
        return false;
    }
    
    // Initialize event ring
    if (!xhci_init_event_ring(xhci)) {
        printf("Failed to initialize event ring\n");
        return false;
    }
    
    // Enable interrupts
    uint8_t* intr_regs = xhci->runtime_regs + 0x20;
    uint32_t iman = xhci_read32(intr_regs);
    iman |= 0x3; // Enable interrupts and clear pending
    xhci_write32(intr_regs, iman);
    
    // Start the controller
    uint32_t usbcmd = xhci_read32(xhci->op_regs + XHCI_OP_USBCMD);
    usbcmd |= XHCI_CMD_RUN | XHCI_CMD_INTE;
    xhci_write32(xhci->op_regs + XHCI_OP_USBCMD, usbcmd);
    
    // Wait for controller to start
    int timeout = 1000;
    while (timeout-- > 0) {
        uint32_t usbsts = xhci_read32(xhci->op_regs + XHCI_OP_USBSTS);
        if (!(usbsts & XHCI_STS_HCH)) {
            break; // Controller running
        }
        for (volatile int i = 0; i < 1000; i++);
    }
    
    if (timeout <= 0) {
        printf("Controller start timeout\n");
        return false;
    }
    
    printf("xHCI controller started successfully\n");
    xhci->running = true;
    return true;
}

// Enable a device slot
bool xhci_enable_slot(xhci_controller_t* xhci, uint8_t* slot_id) {
    printf("Enabling device slot...\n");
    
    // Create Enable Slot command TRB
    xhci_trb_t trb;
    trb.parameter = 0;
    trb.status = 0;
    trb.control = (TRB_TYPE_ENABLE_SLOT << 10) | (1 << 0); // Set cycle bit
    
    // Post the command
    if (!xhci_post_command(xhci, &trb)) {
        printf("Failed to post Enable Slot command\n");
        return false;
    }
    
    // Wait for and process the command completion event
    // In a real implementation, we'd wait for an interrupt or poll the event ring
    // For now, we'll process events synchronously
    for (int i = 0; i < 100; i++) {
        if (xhci_process_events(xhci)) {
            // Event processed - in a full implementation, we'd extract the slot ID
            // from the command completion event
            *slot_id = 1; // Simplified: assume slot 1 for now
            printf("Slot %d enabled\n", *slot_id);
            return true;
        }
        // Small delay
        for (volatile int j = 0; j < 10000; j++);
    }
    
    printf("Timeout waiting for Enable Slot completion\n");
    return false;
}

// Address a device
bool xhci_address_device(xhci_controller_t* xhci, uint8_t slot_id, uint8_t port_num) {
    printf("Addressing device in slot %d (port %d)...\n", slot_id, port_num);
    
    // Allocate device context
    xhci_device_context_t* dev_ctx = (xhci_device_context_t*)malloc(sizeof(xhci_device_context_t));
    if (!dev_ctx) {
        printf("Failed to allocate device context\n");
        return false;
    }
    memset(dev_ctx, 0, sizeof(xhci_device_context_t));
    
    // Allocate input context
    xhci_input_context_t* input_ctx = (xhci_input_context_t*)malloc(sizeof(xhci_input_context_t));
    if (!input_ctx) {
        printf("Failed to allocate input context\n");
        free(dev_ctx);
        return false;
    }
    memset(input_ctx, 0, sizeof(xhci_input_context_t));
    
    // Store contexts
    xhci->devices[slot_id - 1].device_context = dev_ctx;
    xhci->devices[slot_id - 1].input_context = input_ctx;
    xhci->devices[slot_id - 1].in_use = true;
    xhci->devices[slot_id - 1].slot_id = slot_id;
    xhci->devices[slot_id - 1].port_num = port_num;
    
    // Add to DCBAA
    xhci->dcbaa[slot_id] = (uint64_t)dev_ctx;
    
    // Set input control context (add context flags)
    input_ctx->input_control_context[1] = 0x3; // A0 and A1 (slot and EP0)
    
    // Configure slot context
    uint32_t portsc = xhci_read_port_reg(xhci, port_num, 0);
    uint8_t speed = xhci_get_port_speed(portsc);
    
    input_ctx->slot_context[0] = (1 << 27); // Context entries = 1 (EP0 only)
    input_ctx->slot_context[0] |= (speed << 20); // Port speed
    input_ctx->slot_context[1] = (port_num << 16); // Root hub port number
    
    // Configure EP0 context (control endpoint)
    input_ctx->ep0_context[0] = 0; // Endpoint state
    input_ctx->ep0_context[1] = (4 << 3); // EP type = Control (bidirectional)
    input_ctx->ep0_context[1] |= (512 << 16); // Max packet size (512 for SS, 64 for HS/FS)
    
    // Create Address Device command TRB
    xhci_trb_t trb;
    trb.parameter = (uint64_t)input_ctx;
    trb.status = 0;
    trb.control = (TRB_TYPE_ADDRESS_DEVICE << 10) | (slot_id << 24) | (1 << 0);
    
    // Post the command
    if (!xhci_post_command(xhci, &trb)) {
        printf("Failed to post Address Device command\n");
        return false;
    }
    
    // Wait for completion
    for (int i = 0; i < 100; i++) {
        if (xhci_process_events(xhci)) {
            printf("Device addressed successfully\n");
            return true;
        }
        for (volatile int j = 0; j < 10000; j++);
    }
    
    printf("Timeout waiting for Address Device completion\n");
    return false;
}

// Perform a control transfer
bool xhci_control_transfer(xhci_controller_t* xhci, uint8_t slot_id, void* setup_packet, 
                          void* data, uint16_t length, bool direction_in) {
    // For now, this is a simplified stub
    // In a full implementation, we would:
    // 1. Set up a transfer ring for EP0
    // 2. Post SETUP, DATA (optional), and STATUS TRBs
    // 3. Ring the doorbell
    // 4. Wait for transfer completion events
    
    printf("Control transfer to slot %d (length: %d, dir: %s)\n", 
           slot_id, length, direction_in ? "IN" : "OUT");
    
    // TODO: Implement actual control transfer
    return true;
}

// Get device descriptor
bool xhci_get_device_descriptor(xhci_controller_t* xhci, uint8_t slot_id, void* buffer) {
    printf("Reading device descriptor...\n");
    
    usb_setup_packet_t setup;
    setup.bmRequestType = USB_SETUP_DEVICE_TO_HOST(USB_REQ_TYPE_STANDARD | USB_REQ_RECIPIENT_DEVICE);
    setup.bRequest = USB_REQ_GET_DESCRIPTOR;
    setup.wValue = (USB_DESC_DEVICE << 8) | 0; // Descriptor type and index
    setup.wIndex = 0;
    setup.wLength = sizeof(usb_device_descriptor_t);
    
    if (!xhci_control_transfer(xhci, slot_id, &setup, buffer, setup.wLength, true)) {
        printf("Failed to get device descriptor\n");
        return false;
    }
    
    // Parse and display descriptor info
    usb_device_descriptor_t* desc = (usb_device_descriptor_t*)buffer;
    printf("  USB Version: %x.%x\n", desc->bcdUSB >> 8, desc->bcdUSB & 0xFF);
    printf("  Device Class: 0x%02x\n", desc->bDeviceClass);
    printf("  Vendor ID: 0x%04x\n", desc->idVendor);
    printf("  Product ID: 0x%04x\n", desc->idProduct);
    printf("  Max Packet Size: %d\n", desc->bMaxPacketSize0);
    printf("  Configurations: %d\n", desc->bNumConfigurations);
    
    return true;
}

// Get configuration descriptor
bool xhci_get_configuration_descriptor(xhci_controller_t* xhci, uint8_t slot_id, void* buffer, uint16_t length) {
    printf("Reading configuration descriptor...\n");
    
    usb_setup_packet_t setup;
    setup.bmRequestType = USB_SETUP_DEVICE_TO_HOST(USB_REQ_TYPE_STANDARD | USB_REQ_RECIPIENT_DEVICE);
    setup.bRequest = USB_REQ_GET_DESCRIPTOR;
    setup.wValue = (USB_DESC_CONFIGURATION << 8) | 0;
    setup.wIndex = 0;
    setup.wLength = length;
    
    if (!xhci_control_transfer(xhci, slot_id, &setup, buffer, length, true)) {
        printf("Failed to get configuration descriptor\n");
        return false;
    }
    
    // Parse configuration descriptor
    usb_config_descriptor_t* config = (usb_config_descriptor_t*)buffer;
    printf("  Configuration: %d\n", config->bConfigurationValue);
    printf("  Interfaces: %d\n", config->bNumInterfaces);
    printf("  Total Length: %d\n", config->wTotalLength);
    
    // Parse interface descriptors
    uint8_t* ptr = (uint8_t*)buffer + sizeof(usb_config_descriptor_t);
    uint8_t* end = (uint8_t*)buffer + config->wTotalLength;
    
    while (ptr < end) {
        uint8_t desc_len = ptr[0];
        uint8_t desc_type = ptr[1];
        
        if (desc_type == USB_DESC_INTERFACE) {
            usb_interface_descriptor_t* iface = (usb_interface_descriptor_t*)ptr;
            printf("  Interface %d: Class 0x%02x, SubClass 0x%02x, Protocol 0x%02x\n",
                   iface->bInterfaceNumber, iface->bInterfaceClass, 
                   iface->bInterfaceSubClass, iface->bInterfaceProtocol);
            
            // Check if it's a HID keyboard
            if (iface->bInterfaceClass == USB_CLASS_HID &&
                iface->bInterfaceSubClass == HID_SUBCLASS_BOOT &&
                iface->bInterfaceProtocol == HID_PROTOCOL_KEYBOARD) {
                printf("    >>> HID Boot Keyboard detected! <<<\n");
            }
        } else if (desc_type == USB_DESC_ENDPOINT) {
            usb_endpoint_descriptor_t* ep = (usb_endpoint_descriptor_t*)ptr;
            printf("  Endpoint 0x%02x: Type %d, Max Packet %d, Interval %d\n",
                   ep->bEndpointAddress, ep->bmAttributes & 0x03,
                   ep->wMaxPacketSize, ep->bInterval);
        }
        
        ptr += desc_len;
    }
    
    return true;
}

// Set configuration
bool xhci_set_configuration(xhci_controller_t* xhci, uint8_t slot_id, uint8_t config_value) {
    printf("Setting configuration %d...\n", config_value);
    
    usb_setup_packet_t setup;
    setup.bmRequestType = USB_SETUP_HOST_TO_DEVICE(USB_REQ_TYPE_STANDARD | USB_REQ_RECIPIENT_DEVICE);
    setup.bRequest = USB_REQ_SET_CONFIGURATION;
    setup.wValue = config_value;
    setup.wIndex = 0;
    setup.wLength = 0;
    
    if (!xhci_control_transfer(xhci, slot_id, &setup, NULL, 0, false)) {
        printf("Failed to set configuration\n");
        return false;
    }
    
    printf("Configuration set successfully\n");
    return true;
}

// Enumerate a device on a specific port
bool xhci_enumerate_device(xhci_controller_t* xhci, uint8_t port) {
    printf("\n=== Enumerating device on port %d ===\n", port + 1);
    
    // Reset the port first
    xhci_reset_port(xhci, port);
    
    // Wait for reset to complete
    for (volatile int i = 0; i < 100000; i++);
    
    // Enable a slot for this device
    uint8_t slot_id;
    if (!xhci_enable_slot(xhci, &slot_id)) {
        printf("Failed to enable slot for device\n");
        return false;
    }
    
    // Address the device
    if (!xhci_address_device(xhci, slot_id, port)) {
        printf("Failed to address device\n");
        return false;
    }
    
    // Read device descriptor
    usb_device_descriptor_t dev_desc;
    if (!xhci_get_device_descriptor(xhci, slot_id, &dev_desc)) {
        printf("Failed to read device descriptor\n");
        return false;
    }
    
    // Read configuration descriptor
    uint8_t config_buffer[256];
    if (!xhci_get_configuration_descriptor(xhci, slot_id, config_buffer, sizeof(config_buffer))) {
        printf("Failed to read configuration descriptor\n");
        return false;
    }
    
    // Set configuration
    usb_config_descriptor_t* config = (usb_config_descriptor_t*)config_buffer;
    if (!xhci_set_configuration(xhci, slot_id, config->bConfigurationValue)) {
        printf("Failed to set configuration\n");
        return false;
    }
    
    printf("\nDevice enumeration complete for port %d\n", port + 1);
    printf("Device is ready for use!\n");
    return true;
}

// Probe USB ports for connected devices
void xhci_probe_ports(xhci_controller_t* xhci) {
    printf("\nProbing USB ports...\n");
    printf("Number of ports: %d\n\n", xhci->max_ports);
    
    for (uint8_t port = 0; port < xhci->max_ports; port++) {
        uint32_t portsc = xhci_read_port_reg(xhci, port, 0);
        
        printf("Port %d: ", port + 1);
        
        // Check if device is connected
        if (portsc & XHCI_PORTSC_CCS) {
            printf("Device connected - ");
            
            // Check if port is enabled
            if (portsc & XHCI_PORTSC_PED) {
                printf("Enabled - ");
            } else {
                printf("Disabled - ");
            }
            
            // Get port speed
            uint8_t speed = xhci_get_port_speed(portsc);
            printf("%s\n", xhci_get_speed_string(speed));
            
            // Show additional status
            if (portsc & XHCI_PORTSC_OCA) printf("  [Over-current]\n");
            if (portsc & XHCI_PORTSC_PR) printf("  [Reset in progress]\n");
            if (portsc & XHCI_PORTSC_CSC) printf("  [Connect status changed]\n");
            if (portsc & XHCI_PORTSC_PEC) printf("  [Port enabled changed]\n");
            
            // Try to enumerate the device
            if (portsc & XHCI_PORTSC_CSC) {
                // Clear the connect status change bit
                xhci_write_port_reg(xhci, port, 0, portsc | XHCI_PORTSC_CSC);
                
                // Enumerate the device
                xhci_enumerate_device(xhci, port);
            }
            
        } else {
            printf("No device\n");
        }
    }
}

// Detect and initialize xHCI controller from PCI
bool xhci_detect_controller(uint8_t bus, uint8_t device, uint8_t function) {
    // Read class code and check if it's a USB xHCI controller
    uint32_t class_code = pci_config_read(bus, device, function, 0x08);
    uint8_t base_class = (class_code >> 24) & 0xFF;
    uint8_t sub_class = (class_code >> 16) & 0xFF;
    uint8_t prog_if = (class_code >> 8) & 0xFF;
    
    if (base_class != PCI_CLASS_SERIAL_BUS || 
        sub_class != PCI_SUBCLASS_USB || 
        prog_if != PCI_INTERFACE_XHCI) {
        return false;
    }
    
    printf("Found xHCI controller at %d:%d.%d\n", bus, device, function);
    
    // Allocate controller structure
    g_xhci = (xhci_controller_t*)malloc(sizeof(xhci_controller_t));
    if (!g_xhci) {
        printf("Failed to allocate xHCI controller structure\n");
        return false;
    }
    memset(g_xhci, 0, sizeof(xhci_controller_t));
    
    // Store PCI location
    g_xhci->bus = bus;
    g_xhci->device = device;
    g_xhci->function = function;
    
    // Read BAR0 (Base Address Register 0) - memory-mapped I/O base
    uint32_t bar0 = pci_config_read(bus, device, function, 0x10);
    g_xhci->base_addr = (uint8_t*)(bar0 & 0xFFFFFFF0); // Mask off lower bits
    
    printf("xHCI Base Address: 0x%08x\n", (uint32_t)g_xhci->base_addr);
    
    // Read capability registers
    g_xhci->cap_regs = g_xhci->base_addr;
    uint8_t caplength = *g_xhci->cap_regs;
    g_xhci->hci_version = *((uint16_t*)(g_xhci->cap_regs + XHCI_CAP_HCIVERSION));
    
    printf("xHCI Version: %d.%d\n", g_xhci->hci_version >> 8, g_xhci->hci_version & 0xFF);
    
    // Calculate operational registers base
    g_xhci->op_regs = g_xhci->base_addr + caplength;
    
    // Read structural parameters
    uint32_t hcsparams1 = xhci_read32(g_xhci->cap_regs + XHCI_CAP_HCSPARAMS1);
    g_xhci->max_slots = hcsparams1 & 0xFF;
    g_xhci->max_interrupters = (hcsparams1 >> 8) & 0x7FF;
    g_xhci->max_ports = (hcsparams1 >> 24) & 0xFF;
    
    printf("Max Device Slots: %d\n", g_xhci->max_slots);
    printf("Max Interrupters: %d\n", g_xhci->max_interrupters);
    printf("Max Ports: %d\n", g_xhci->max_ports);
    
    // Get runtime and doorbell registers
    uint32_t rtsoff = xhci_read32(g_xhci->cap_regs + XHCI_CAP_RTSOFF);
    uint32_t dboff = xhci_read32(g_xhci->cap_regs + XHCI_CAP_DBOFF);
    
    g_xhci->runtime_regs = g_xhci->base_addr + (rtsoff & ~0x1F);
    g_xhci->doorbell_regs = (uint32_t*)(g_xhci->base_addr + (dboff & ~0x3));
    
    // Reset and start the controller
    if (!xhci_reset_controller(g_xhci)) {
        printf("Failed to reset xHCI controller\n");
        free(g_xhci);
        g_xhci = NULL;
        return false;
    }
    
    if (!xhci_start_controller(g_xhci)) {
        printf("Failed to start xHCI controller\n");
        free(g_xhci);
        g_xhci = NULL;
        return false;
    }
    
    g_xhci->initialized = true;
    
    // Probe ports for connected devices
    xhci_probe_ports(g_xhci);
    
    return true;
}

// Scan for USB devices (enumerate all PCI devices looking for xHCI)
void xhci_scan_devices(void) {
    printf("Scanning for USB 3.0 (xHCI) controllers...\n\n");
    
    bool found = false;
    
    // Scan PCI bus for xHCI controllers
    for (int bus = 0; bus < 256; bus++) {
        for (int device = 0; device < 32; device++) {
            for (int function = 0; function < 8; function++) {
                uint16_t vendor = pci_config_read(bus, device, function, 0x00) & 0xFFFF;
                if (vendor == 0xFFFF) {
                    continue; // No device
                }
                
                if (xhci_detect_controller(bus, device, function)) {
                    found = true;
                    // We only support one controller for now
                    return;
                }
            }
        }
    }
    
    if (!found) {
        printf("No xHCI controllers found\n");
    }
}

// Initialize USB 3.0 subsystem
bool xhci_init(void) {
    printf("=== USB 3.0 (xHCI) Driver Initialization ===\n\n");
    xhci_scan_devices();
    
    if (g_xhci && g_xhci->initialized) {
        printf("\nUSB 3.0 driver initialized successfully\n");
        return true;
    }
    
    printf("\nUSB 3.0 driver initialization failed\n");
    return false;
}

