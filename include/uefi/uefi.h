#ifndef UEFI_H
#define UEFI_H

#if defined(__x86_64__)
#define EFIAPI __attribute__((ms_abi))
#else
#define EFIAPI
#endif

#include <stdint.h>

// UEFI Status type
typedef uint64_t EFI_STATUS;

// UEFI Status Codes
#define EFI_SUCCESS                     0
#define EFI_LOAD_ERROR                  (1 | (1 << 31))
#define EFI_INVALID_PARAMETER           (2 | (1 << 31))
#define EFI_UNSUPPORTED                 (3 | (1 << 31))
#define EFI_BAD_BUFFER_SIZE             (4 | (1 << 31))
#define EFI_BUFFER_TOO_SMALL            (5 | (1 << 31))
#define EFI_NOT_READY                   (6 | (1 << 31))
#define EFI_DEVICE_ERROR                (7 | (1 << 31))
#define EFI_WRITE_PROTECTED             (8 | (1 << 31))
#define EFI_OUT_OF_RESOURCES            (9 | (1 << 31))
#define EFI_VOLUME_CORRUPTED            (10 | (1 << 31))
#define EFI_VOLUME_FULL                 (11 | (1 << 31))
#define EFI_NO_MEDIA                    (12 | (1 << 31))
#define EFI_MEDIA_CHANGED               (13 | (1 << 31))
#define EFI_NOT_FOUND                   (14 | (1 << 31))
#define EFI_ACCESS_DENIED               (15 | (1 << 31))
#define EFI_NO_RESPONSE                 (16 | (1 << 31))
#define EFI_NO_MAPPING                  (17 | (1 << 31))
#define EFI_TIMEOUT                     (18 | (1 << 31))
#define EFI_NOT_STARTED                 (19 | (1 << 31))
#define EFI_ALREADY_STARTED             (20 | (1 << 31))
#define EFI_ABORTED                     (21 | (1 << 31))
#define EFI_ICMP_ERROR                  (22 | (1 << 31))
#define EFI_TFTP_ERROR                  (23 | (1 << 31))
#define EFI_PROTOCOL_ERROR              (24 | (1 << 31))
#define EFI_INCOMPATIBLE_VERSION        (25 | (1 << 31))
#define EFI_SECURITY_VIOLATION          (26 | (1 << 31))
#define EFI_CRC_ERROR                   (27 | (1 << 31))
#define EFI_END_OF_MEDIA                (28 | (1 << 31))
#define EFI_END_OF_FILE                 (31 | (1 << 31))
#define EFI_INVALID_LANGUAGE            (32 | (1 << 31))
#define EFI_COMPROMISED_DATA            (33 | (1 << 31))
#define EFI_IP_ADDRESS_CONFLICT         (34 | (1 << 31))
#define EFI_HTTP_ERROR                  (35 | (1 << 31))

// EFI Error checking macro
#define EFI_ERROR(Status) ((Status) != EFI_SUCCESS)

// UEFI Data Types
typedef uint64_t EFI_PHYSICAL_ADDRESS;
typedef uint64_t EFI_VIRTUAL_ADDRESS;
typedef uint64_t EFI_LBA;
typedef uint64_t EFI_TPL;
typedef uint64_t EFI_MEMORY_TYPE;
typedef uint64_t EFI_ALLOCATE_TYPE;
typedef uint64_t EFI_TIMER_DELAY;
typedef uint64_t EFI_EVENT_NOTIFY;
typedef uint64_t EFI_RAISE_TPL;
typedef uint64_t EFI_RESTORE_TPL;
typedef uint64_t EFI_ALLOCATE_PAGES;
typedef uint64_t EFI_FREE_PAGES;
typedef uint64_t EFI_GET_MEMORY_MAP;
typedef uint64_t EFI_ALLOCATE_POOL;
typedef uint64_t EFI_FREE_POOL;
typedef uint64_t EFI_CREATE_EVENT;
typedef uint64_t EFI_SET_TIMER;
typedef uint64_t EFI_WAIT_FOR_EVENT;
typedef uint64_t EFI_SIGNAL_EVENT;
typedef uint64_t EFI_CLOSE_EVENT;
typedef uint64_t EFI_CHECK_EVENT;
typedef uint64_t EFI_INSTALL_PROTOCOL_INTERFACE;
typedef uint64_t EFI_REINSTALL_PROTOCOL_INTERFACE;
typedef uint64_t EFI_UNINSTALL_PROTOCOL_INTERFACE;
typedef uint64_t EFI_HANDLE_PROTOCOL;
typedef uint64_t EFI_PC_HANDLE_PROTOCOL;
typedef uint64_t EFI_REGISTER_PROTOCOL_NOTIFY;
typedef uint64_t EFI_LOCATE_HANDLE;
typedef uint64_t EFI_LOCATE_DEVICE_PATH;
typedef uint64_t EFI_INSTALL_CONFIGURATION_TABLE;
typedef uint64_t EFI_IMAGE_LOAD;
typedef uint64_t EFI_IMAGE_START;
typedef uint64_t EFI_EXIT;
typedef uint64_t EFI_IMAGE_UNLOAD;
typedef uint64_t EFI_EXIT_BOOT_SERVICES;
typedef uint64_t EFI_GET_NEXT_MONOTONIC_COUNT;
typedef uint64_t EFI_STALL;
typedef uint64_t EFI_SET_WATCHDOG_TIMER;
typedef uint64_t EFI_CONNECT_CONTROLLER;
typedef uint64_t EFI_DISCONNECT_CONTROLLER;
typedef uint64_t EFI_OPEN_PROTOCOL;
typedef uint64_t EFI_CLOSE_PROTOCOL;
typedef uint64_t EFI_OPEN_PROTOCOL_INFORMATION;
typedef uint64_t EFI_PROTOCOLS_PER_HANDLE;
typedef uint64_t EFI_LOCATE_HANDLE_BUFFER;
typedef uint64_t EFI_LOCATE_PROTOCOL;
typedef uint64_t EFI_INSTALL_MULTIPLE_PROTOCOL_INTERFACES;
typedef uint64_t EFI_UNINSTALL_MULTIPLE_PROTOCOL_INTERFACES;
typedef uint64_t EFI_CALCULATE_CRC32;
typedef uint64_t EFI_COPY_MEM;
typedef uint64_t EFI_SET_MEM;
typedef uint64_t EFI_CREATE_EVENT_EX;
typedef uint64_t EFI_GET_TIME;
typedef uint64_t EFI_SET_TIME;
typedef uint64_t EFI_GET_WAKEUP_TIME;
typedef uint64_t EFI_SET_WAKEUP_TIME;
typedef uint64_t EFI_SET_VIRTUAL_ADDRESS_MAP;
typedef uint64_t EFI_CONVERT_POINTER;
typedef uint64_t EFI_GET_VARIABLE;
typedef uint64_t EFI_GET_NEXT_VARIABLE_NAME;
typedef uint64_t EFI_SET_VARIABLE;
typedef uint64_t EFI_GET_NEXT_HIGH_MONOTONIC_COUNT;
typedef uint64_t EFI_RESET_SYSTEM;
typedef uint64_t EFI_UPDATE_CAPSULE;
typedef uint64_t EFI_QUERY_CAPSULE_CAPABILITIES;
typedef uint64_t EFI_QUERY_VARIABLE_INFO;

// UEFI Handle
typedef void* EFI_HANDLE;

// UEFI Event
typedef void* EFI_EVENT;

// UEFI GUID
typedef struct {
    uint32_t Data1;
    uint16_t Data2;
    uint16_t Data3;
    uint8_t Data4[8];
} EFI_GUID;

// UEFI Time
typedef struct {
    uint16_t Year;
    uint8_t Month;
    uint8_t Day;
    uint8_t Hour;
    uint8_t Minute;
    uint8_t Second;
    uint8_t Pad1;
    uint32_t Nanosecond;
    int16_t TimeZone;
    uint8_t Daylight;
    uint8_t Pad2;
} EFI_TIME;

// UEFI Time Capabilities
typedef struct {
    uint32_t Resolution;
    uint32_t Accuracy;
    uint8_t SetsToZero;
} EFI_TIME_CAPABILITIES;

// UEFI Memory Descriptor
typedef struct {
    uint32_t Type;
    EFI_PHYSICAL_ADDRESS PhysicalStart;
    EFI_VIRTUAL_ADDRESS VirtualStart;
    uint64_t NumberOfPages;
    uint64_t Attribute;
} EFI_MEMORY_DESCRIPTOR;

// UEFI Memory Map
typedef struct {
    EFI_MEMORY_DESCRIPTOR* MemoryMap;
    uint64_t MapSize;
    uint64_t MapKey;
    uint64_t DescriptorSize;
    uint32_t DescriptorVersion;
} EFI_MEMORY_MAP;

// UEFI Input Key
typedef struct {
    uint16_t ScanCode;
    uint16_t UnicodeChar;
} EFI_INPUT_KEY;

// UEFI Mode
typedef struct {
    int32_t MaxMode;
    int32_t Mode;
    int32_t Attribute;
    int32_t CursorColumn;
    int32_t CursorRow;
    uint8_t CursorVisible;
} EFI_MODE;

// Boolean type
typedef uint8_t BOOLEAN;
#define TRUE 1
#define FALSE 0

// Forward declarations
struct _EFI_SIMPLE_TEXT_INPUT_PROTOCOL;
struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

// UEFI Table Header
typedef struct {
    uint64_t Signature;
    uint32_t Revision;
    uint32_t HeaderSize;
    uint32_t CRC32;
    uint32_t Reserved;
} EFI_TABLE_HEADER;

// UEFI Configuration Table
typedef struct {
    EFI_GUID VendorGuid;
    void* VendorTable;
} EFI_CONFIGURATION_TABLE;

// Protocol function pointer types
typedef EFI_STATUS (EFIAPI *EFI_INPUT_RESET)(struct _EFI_SIMPLE_TEXT_INPUT_PROTOCOL *This, BOOLEAN ExtendedVerification);
typedef EFI_STATUS (EFIAPI *EFI_INPUT_READ_KEY)(struct _EFI_SIMPLE_TEXT_INPUT_PROTOCOL *This, EFI_INPUT_KEY *Key);

typedef EFI_STATUS (EFIAPI *EFI_TEXT_RESET)(struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, BOOLEAN ExtendedVerification);
typedef EFI_STATUS (EFIAPI *EFI_TEXT_STRING)(struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, uint16_t *String);
typedef EFI_STATUS (EFIAPI *EFI_TEXT_TEST_STRING)(struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, uint16_t *String);
typedef EFI_STATUS (EFIAPI *EFI_TEXT_QUERY_MODE)(struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, uint64_t ModeNumber, uint64_t *Columns, uint64_t *Rows);
typedef EFI_STATUS (EFIAPI *EFI_TEXT_SET_MODE)(struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, uint64_t ModeNumber);
typedef EFI_STATUS (EFIAPI *EFI_TEXT_SET_ATTRIBUTE)(struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, uint64_t Attribute);
typedef EFI_STATUS (EFIAPI *EFI_TEXT_CLEAR_SCREEN)(struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This);
typedef EFI_STATUS (EFIAPI *EFI_TEXT_SET_CURSOR_POSITION)(struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, uint64_t Column, uint64_t Row);
typedef EFI_STATUS (EFIAPI *EFI_TEXT_ENABLE_CURSOR)(struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, BOOLEAN Visible);

// UEFI Simple Text Input Protocol
typedef struct _EFI_SIMPLE_TEXT_INPUT_PROTOCOL {
    EFI_INPUT_RESET Reset;
    EFI_INPUT_READ_KEY ReadKeyStroke;
    EFI_EVENT WaitForKey;
} EFI_SIMPLE_TEXT_INPUT_PROTOCOL;

// UEFI Simple Text Output Protocol
typedef struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
    EFI_TEXT_RESET Reset;
    EFI_TEXT_STRING OutputString;
    EFI_TEXT_TEST_STRING TestString;
    EFI_TEXT_QUERY_MODE QueryMode;
    EFI_TEXT_SET_MODE SetMode;
    EFI_TEXT_SET_ATTRIBUTE SetAttribute;
    EFI_TEXT_CLEAR_SCREEN ClearScreen;
    EFI_TEXT_SET_CURSOR_POSITION SetCursorPosition;
    EFI_TEXT_ENABLE_CURSOR EnableCursor;
    EFI_MODE* Mode;
} EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

// Boot Services function pointer types
typedef EFI_STATUS (EFIAPI *EFI_BOOT_STALL)(uint64_t Microseconds);
typedef EFI_STATUS (EFIAPI *EFI_BOOT_EXIT)(EFI_HANDLE ImageHandle, EFI_STATUS ExitStatus, uint64_t ExitDataSize, uint16_t *ExitData);

// UEFI Boot Services
typedef struct {
    EFI_TABLE_HEADER Hdr;
    void* RaiseTPL;
    void* RestoreTPL;
    void* AllocatePages;
    void* FreePages;
    void* GetMemoryMap;
    void* AllocatePool;
    void* FreePool;
    void* CreateEvent;
    void* SetTimer;
    void* WaitForEvent;
    void* SignalEvent;
    void* CloseEvent;
    void* CheckEvent;
    void* InstallProtocolInterface;
    void* ReinstallProtocolInterface;
    void* UninstallProtocolInterface;
    void* HandleProtocol;
    void* PC_HandleProtocol;
    void* RegisterProtocolNotify;
    void* LocateHandle;
    void* LocateDevicePath;
    void* InstallConfigurationTable;
    void* LoadImage;
    void* StartImage;
    EFI_BOOT_EXIT Exit;
    void* UnloadImage;
    void* ExitBootServices;
    void* GetNextMonotonicCount;
    EFI_BOOT_STALL Stall;
    void* SetWatchdogTimer;
    void* ConnectController;
    void* DisconnectController;
    void* OpenProtocol;
    void* CloseProtocol;
    void* OpenProtocolInformation;
    void* ProtocolsPerHandle;
    void* LocateHandleBuffer;
    void* LocateProtocol;
    void* InstallMultipleProtocolInterfaces;
    void* UninstallMultipleProtocolInterfaces;
    void* CalculateCrc32;
    void* CopyMem;
    void* SetMem;
    void* CreateEventEx;
} EFI_BOOT_SERVICES;

// UEFI Runtime Services
typedef struct {
    EFI_TABLE_HEADER Hdr;
    void* GetTime;
    void* SetTime;
    void* GetWakeupTime;
    void* SetWakeupTime;
    void* SetVirtualAddressMap;
    void* ConvertPointer;
    void* GetVariable;
    void* GetNextVariableName;
    void* SetVariable;
    void* GetNextHighMonotonicCount;
    void* ResetSystem;
    void* UpdateCapsule;
    void* QueryCapsuleCapabilities;
    void* QueryVariableInfo;
} EFI_RUNTIME_SERVICES;

// UEFI System Table
typedef struct {
    EFI_TABLE_HEADER Hdr;
    uint16_t* FirmwareVendor;
    uint32_t FirmwareRevision;
    EFI_HANDLE ConsoleInHandle;
    EFI_SIMPLE_TEXT_INPUT_PROTOCOL* ConIn;
    EFI_HANDLE ConsoleOutHandle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* ConOut;
    EFI_HANDLE StandardErrorHandle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* StdErr;
    EFI_RUNTIME_SERVICES* RuntimeServices;
    EFI_BOOT_SERVICES* BootServices;
    uint64_t NumberOfTableEntries;
    EFI_CONFIGURATION_TABLE* ConfigurationTable;
} EFI_SYSTEM_TABLE;

// UEFI Application Entry Point
typedef EFI_STATUS (EFIAPI *EFI_IMAGE_ENTRY_POINT)(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable);

#endif // UEFI_H 