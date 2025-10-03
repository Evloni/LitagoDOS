#ifndef XHCI_H
#define XHCI_H

#include <stdint.h>
#include <stdbool.h>

// xHCI PCI Class Codes
#define PCI_CLASS_SERIAL_BUS    0x0C
#define PCI_SUBCLASS_USB        0x03
#define PCI_INTERFACE_XHCI      0x30

// xHCI Capability Register Offsets
#define XHCI_CAP_CAPLENGTH      0x00
#define XHCI_CAP_HCIVERSION     0x02
#define XHCI_CAP_HCSPARAMS1     0x04
#define XHCI_CAP_HCSPARAMS2     0x08
#define XHCI_CAP_HCSPARAMS3     0x0C
#define XHCI_CAP_HCCPARAMS1     0x10
#define XHCI_CAP_DBOFF          0x14
#define XHCI_CAP_RTSOFF         0x18

// xHCI Operational Register Offsets (relative to operational base)
#define XHCI_OP_USBCMD          0x00
#define XHCI_OP_USBSTS          0x04
#define XHCI_OP_PAGESIZE        0x08
#define XHCI_OP_DNCTRL          0x14
#define XHCI_OP_CRCR            0x18
#define XHCI_OP_DCBAAP          0x30
#define XHCI_OP_CONFIG          0x38

// xHCI USB Command Register (USBCMD) bits
#define XHCI_CMD_RUN            (1 << 0)    // Run/Stop
#define XHCI_CMD_HCRST          (1 << 1)    // Host Controller Reset
#define XHCI_CMD_INTE           (1 << 2)    // Interrupter Enable
#define XHCI_CMD_HSEE           (1 << 3)    // Host System Error Enable
#define XHCI_CMD_LHCRST         (1 << 7)    // Light Host Controller Reset
#define XHCI_CMD_CSS            (1 << 8)    // Controller Save State
#define XHCI_CMD_CRS            (1 << 9)    // Controller Restore State
#define XHCI_CMD_EWE            (1 << 10)   // Enable Wrap Event
#define XHCI_CMD_EU3S           (1 << 11)   // Enable U3 MFINDEX Stop

// xHCI USB Status Register (USBSTS) bits
#define XHCI_STS_HCH            (1 << 0)    // HC Halted
#define XHCI_STS_HSE            (1 << 2)    // Host System Error
#define XHCI_STS_EINT           (1 << 3)    // Event Interrupt
#define XHCI_STS_PCD            (1 << 4)    // Port Change Detect
#define XHCI_STS_SSS            (1 << 8)    // Save State Status
#define XHCI_STS_RSS            (1 << 9)    // Restore State Status
#define XHCI_STS_SRE            (1 << 10)   // Save/Restore Error
#define XHCI_STS_CNR            (1 << 11)   // Controller Not Ready
#define XHCI_STS_HCE            (1 << 12)   // Host Controller Error

// Port Status and Control Register bits
#define XHCI_PORTSC_CCS         (1 << 0)    // Current Connect Status
#define XHCI_PORTSC_PED         (1 << 1)    // Port Enabled/Disabled
#define XHCI_PORTSC_OCA         (1 << 3)    // Over-current Active
#define XHCI_PORTSC_PR          (1 << 4)    // Port Reset
#define XHCI_PORTSC_PP          (1 << 9)    // Port Power
#define XHCI_PORTSC_SPEED_MASK  (0xF << 10) // Port Speed
#define XHCI_PORTSC_PIC_MASK    (0x3 << 14) // Port Indicator Control
#define XHCI_PORTSC_LWS         (1 << 16)   // Port Link State Write Strobe
#define XHCI_PORTSC_CSC         (1 << 17)   // Connect Status Change
#define XHCI_PORTSC_PEC         (1 << 18)   // Port Enabled/Disabled Change
#define XHCI_PORTSC_WRC         (1 << 19)   // Warm Port Reset Change
#define XHCI_PORTSC_OCC         (1 << 20)   // Over-current Change
#define XHCI_PORTSC_PRC         (1 << 21)   // Port Reset Change
#define XHCI_PORTSC_PLC         (1 << 22)   // Port Link State Change
#define XHCI_PORTSC_CEC         (1 << 23)   // Port Config Error Change
#define XHCI_PORTSC_CAS         (1 << 24)   // Cold Attach Status
#define XHCI_PORTSC_WCE         (1 << 25)   // Wake on Connect Enable
#define XHCI_PORTSC_WDE         (1 << 26)   // Wake on Disconnect Enable
#define XHCI_PORTSC_WOE         (1 << 27)   // Wake on Over-current Enable
#define XHCI_PORTSC_DR          (1 << 30)   // Device Removable
#define XHCI_PORTSC_WPR         (1 << 31)   // Warm Port Reset

// USB Speed Codes
#define XHCI_SPEED_FULL         1
#define XHCI_SPEED_LOW          2
#define XHCI_SPEED_HIGH         3
#define XHCI_SPEED_SUPER        4

// TRB (Transfer Request Block) Types
#define TRB_TYPE_NORMAL         1
#define TRB_TYPE_SETUP          2
#define TRB_TYPE_DATA           3
#define TRB_TYPE_STATUS         4
#define TRB_TYPE_LINK           6
#define TRB_TYPE_EVENT_DATA     7
#define TRB_TYPE_NOOP           8
#define TRB_TYPE_ENABLE_SLOT    9
#define TRB_TYPE_DISABLE_SLOT   10
#define TRB_TYPE_ADDRESS_DEVICE 11
#define TRB_TYPE_CONFIG_EP      12
#define TRB_TYPE_EVAL_CONTEXT   13
#define TRB_TYPE_RESET_EP       14
#define TRB_TYPE_STOP_EP        15
#define TRB_TYPE_SET_TR_DEQUEUE 16
#define TRB_TYPE_RESET_DEVICE   17
#define TRB_TYPE_CMD_COMPLETION 33
#define TRB_TYPE_PORT_STATUS    34
#define TRB_TYPE_TRANSFER       32

// TRB Completion Codes
#define TRB_CC_SUCCESS          1
#define TRB_CC_SHORT_PACKET     13

// Maximum values
#define XHCI_MAX_SLOTS          255
#define XHCI_MAX_PORTS          127
#define XHCI_MAX_INTERRUPTERS   1024

// Structure alignments
#define XHCI_ALIGN_64           64
#define XHCI_ALIGN_16           16

// TRB (Transfer Request Block) structure
typedef struct {
    uint64_t parameter;
    uint32_t status;
    uint32_t control;
} __attribute__((packed)) xhci_trb_t;

// Event Ring Segment Table Entry
typedef struct {
    uint64_t ring_segment_base;
    uint32_t ring_segment_size;
    uint32_t reserved;
} __attribute__((packed)) xhci_erst_entry_t;

// Command Ring structure
typedef struct {
    xhci_trb_t* trbs;
    uint32_t enqueue_ptr;
    uint32_t dequeue_ptr;
    uint32_t cycle_state;
    uint32_t size;
} xhci_command_ring_t;

// Event Ring structure
typedef struct {
    xhci_trb_t* trbs;
    xhci_erst_entry_t* erst;
    uint32_t dequeue_ptr;
    uint32_t cycle_state;
    uint32_t size;
} xhci_event_ring_t;

// Device Context structure (simplified)
typedef struct {
    uint32_t slot_context[8];
    uint32_t ep0_context[8];
    uint32_t ep_context[31][8];  // Up to 31 endpoints
} __attribute__((packed, aligned(64))) xhci_device_context_t;

// Input Context structure
typedef struct {
    uint32_t input_control_context[8];
    uint32_t slot_context[8];
    uint32_t ep0_context[8];
    uint32_t ep_context[31][8];
} __attribute__((packed, aligned(64))) xhci_input_context_t;

// USB Device structure
typedef struct {
    uint8_t slot_id;
    uint8_t port_num;
    uint8_t speed;
    uint8_t address;
    xhci_device_context_t* device_context;
    xhci_input_context_t* input_context;
    bool in_use;
} xhci_device_t;

// xHCI Controller structure
typedef struct {
    // PCI information
    uint8_t bus;
    uint8_t device;
    uint8_t function;
    
    // Memory-mapped registers
    uint8_t* base_addr;
    uint8_t* cap_regs;
    uint8_t* op_regs;
    uint8_t* runtime_regs;
    uint32_t* doorbell_regs;
    
    // Controller capabilities
    uint8_t max_slots;
    uint8_t max_ports;
    uint8_t max_interrupters;
    uint16_t hci_version;
    
    // Command and Event rings
    xhci_command_ring_t cmd_ring;
    xhci_event_ring_t event_ring;
    
    // Device Context Base Address Array
    uint64_t* dcbaa;
    
    // Connected devices
    xhci_device_t devices[XHCI_MAX_SLOTS];
    
    // Controller state
    bool initialized;
    bool running;
} xhci_controller_t;

// Function prototypes
bool xhci_init(void);
bool xhci_detect_controller(uint8_t bus, uint8_t device, uint8_t function);
bool xhci_reset_controller(xhci_controller_t* xhci);
bool xhci_start_controller(xhci_controller_t* xhci);
void xhci_probe_ports(xhci_controller_t* xhci);
void xhci_scan_devices(void);

// Command ring functions
bool xhci_init_command_ring(xhci_controller_t* xhci);
bool xhci_post_command(xhci_controller_t* xhci, xhci_trb_t* trb);
void xhci_ring_doorbell(xhci_controller_t* xhci, uint8_t doorbell, uint8_t target);

// Event ring functions
bool xhci_init_event_ring(xhci_controller_t* xhci);
bool xhci_process_events(xhci_controller_t* xhci);

// Device management functions
bool xhci_enable_slot(xhci_controller_t* xhci, uint8_t* slot_id);
bool xhci_address_device(xhci_controller_t* xhci, uint8_t slot_id, uint8_t port_num);
bool xhci_configure_endpoint(xhci_controller_t* xhci, uint8_t slot_id);
bool xhci_enumerate_device(xhci_controller_t* xhci, uint8_t port);

// Port management functions
uint32_t xhci_read_port_reg(xhci_controller_t* xhci, uint8_t port, uint8_t reg);
void xhci_write_port_reg(xhci_controller_t* xhci, uint8_t port, uint8_t reg, uint32_t value);
void xhci_reset_port(xhci_controller_t* xhci, uint8_t port);

// Utility functions
uint8_t xhci_get_port_speed(uint32_t portsc);
const char* xhci_get_speed_string(uint8_t speed);

// Transfer functions
bool xhci_control_transfer(xhci_controller_t* xhci, uint8_t slot_id, void* setup_packet, 
                          void* data, uint16_t length, bool direction_in);
bool xhci_get_device_descriptor(xhci_controller_t* xhci, uint8_t slot_id, void* buffer);
bool xhci_get_configuration_descriptor(xhci_controller_t* xhci, uint8_t slot_id, void* buffer, uint16_t length);
bool xhci_set_configuration(xhci_controller_t* xhci, uint8_t slot_id, uint8_t config_value);

// Global controller instance
extern xhci_controller_t* g_xhci;

#endif // XHCI_H

