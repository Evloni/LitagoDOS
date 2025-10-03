#ifndef USB_DESCRIPTORS_H
#define USB_DESCRIPTORS_H

#include <stdint.h>

// USB Request Types
#define USB_REQ_TYPE_STANDARD   0x00
#define USB_REQ_TYPE_CLASS      0x20
#define USB_REQ_TYPE_VENDOR     0x40

#define USB_REQ_RECIPIENT_DEVICE    0x00
#define USB_REQ_RECIPIENT_INTERFACE 0x01
#define USB_REQ_RECIPIENT_ENDPOINT  0x02

#define USB_DIR_OUT             0x00
#define USB_DIR_IN              0x80

// USB Standard Requests
#define USB_REQ_GET_STATUS          0x00
#define USB_REQ_CLEAR_FEATURE       0x01
#define USB_REQ_SET_FEATURE         0x03
#define USB_REQ_SET_ADDRESS         0x05
#define USB_REQ_GET_DESCRIPTOR      0x06
#define USB_REQ_SET_DESCRIPTOR      0x07
#define USB_REQ_GET_CONFIGURATION   0x08
#define USB_REQ_SET_CONFIGURATION   0x09
#define USB_REQ_GET_INTERFACE       0x0A
#define USB_REQ_SET_INTERFACE       0x0B

// USB Descriptor Types
#define USB_DESC_DEVICE             0x01
#define USB_DESC_CONFIGURATION      0x02
#define USB_DESC_STRING             0x03
#define USB_DESC_INTERFACE          0x04
#define USB_DESC_ENDPOINT           0x05
#define USB_DESC_DEVICE_QUALIFIER   0x06
#define USB_DESC_HID                0x21
#define USB_DESC_REPORT             0x22
#define USB_DESC_PHYSICAL           0x23

// USB Class Codes
#define USB_CLASS_PER_INTERFACE     0x00
#define USB_CLASS_AUDIO             0x01
#define USB_CLASS_COMM              0x02
#define USB_CLASS_HID               0x03
#define USB_CLASS_PHYSICAL          0x05
#define USB_CLASS_IMAGE             0x06
#define USB_CLASS_PRINTER           0x07
#define USB_CLASS_MASS_STORAGE      0x08
#define USB_CLASS_HUB               0x09
#define USB_CLASS_CDC_DATA          0x0A
#define USB_CLASS_SMART_CARD        0x0B
#define USB_CLASS_CONTENT_SECURITY  0x0D
#define USB_CLASS_VIDEO             0x0E
#define USB_CLASS_WIRELESS          0xE0
#define USB_CLASS_VENDOR_SPECIFIC   0xFF

// HID Subclass and Protocol
#define HID_SUBCLASS_NONE           0x00
#define HID_SUBCLASS_BOOT           0x01

#define HID_PROTOCOL_NONE           0x00
#define HID_PROTOCOL_KEYBOARD       0x01
#define HID_PROTOCOL_MOUSE          0x02

// HID Class Requests
#define HID_REQ_GET_REPORT          0x01
#define HID_REQ_GET_IDLE            0x02
#define HID_REQ_GET_PROTOCOL        0x03
#define HID_REQ_SET_REPORT          0x09
#define HID_REQ_SET_IDLE            0x0A
#define HID_REQ_SET_PROTOCOL        0x0B

// USB Setup Packet
typedef struct {
    uint8_t bmRequestType;
    uint8_t bRequest;
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
} __attribute__((packed)) usb_setup_packet_t;

// USB Device Descriptor
typedef struct {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bcdUSB;
    uint8_t bDeviceClass;
    uint8_t bDeviceSubClass;
    uint8_t bDeviceProtocol;
    uint8_t bMaxPacketSize0;
    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t bcdDevice;
    uint8_t iManufacturer;
    uint8_t iProduct;
    uint8_t iSerialNumber;
    uint8_t bNumConfigurations;
} __attribute__((packed)) usb_device_descriptor_t;

// USB Configuration Descriptor
typedef struct {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t wTotalLength;
    uint8_t bNumInterfaces;
    uint8_t bConfigurationValue;
    uint8_t iConfiguration;
    uint8_t bmAttributes;
    uint8_t bMaxPower;
} __attribute__((packed)) usb_config_descriptor_t;

// USB Interface Descriptor
typedef struct {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bInterfaceNumber;
    uint8_t bAlternateSetting;
    uint8_t bNumEndpoints;
    uint8_t bInterfaceClass;
    uint8_t bInterfaceSubClass;
    uint8_t bInterfaceProtocol;
    uint8_t iInterface;
} __attribute__((packed)) usb_interface_descriptor_t;

// USB Endpoint Descriptor
typedef struct {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bEndpointAddress;
    uint8_t bmAttributes;
    uint16_t wMaxPacketSize;
    uint8_t bInterval;
} __attribute__((packed)) usb_endpoint_descriptor_t;

// HID Descriptor
typedef struct {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bcdHID;
    uint8_t bCountryCode;
    uint8_t bNumDescriptors;
    uint8_t bDescriptorType2;
    uint16_t wDescriptorLength;
} __attribute__((packed)) usb_hid_descriptor_t;

// HID Boot Keyboard Report (8 bytes)
typedef struct {
    uint8_t modifiers;      // Modifier keys (Ctrl, Shift, Alt, etc.)
    uint8_t reserved;       // Reserved (always 0)
    uint8_t keycodes[6];    // Up to 6 simultaneous key presses
} __attribute__((packed)) hid_keyboard_report_t;

// HID Keyboard Modifier Keys
#define HID_MOD_LCTRL   (1 << 0)
#define HID_MOD_LSHIFT  (1 << 1)
#define HID_MOD_LALT    (1 << 2)
#define HID_MOD_LGUI    (1 << 3)
#define HID_MOD_RCTRL   (1 << 4)
#define HID_MOD_RSHIFT  (1 << 5)
#define HID_MOD_RALT    (1 << 6)
#define HID_MOD_RGUI    (1 << 7)

// USB Endpoint attributes
#define USB_EP_ATTR_CONTROL     0x00
#define USB_EP_ATTR_ISOCHRONOUS 0x01
#define USB_EP_ATTR_BULK        0x02
#define USB_EP_ATTR_INTERRUPT   0x03

// Helper macros
#define USB_SETUP_DEVICE_TO_HOST(req_type) ((USB_DIR_IN | (req_type)))
#define USB_SETUP_HOST_TO_DEVICE(req_type) ((USB_DIR_OUT | (req_type)))

#endif // USB_DESCRIPTORS_H

