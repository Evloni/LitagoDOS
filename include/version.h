#ifndef VERSION_H
#define VERSION_H

// Version information
#define VERSION_MAJOR 0
#define VERSION_MINOR 8
#define VERSION_PATCH 0

// Version string
#define VERSION_STRING "0.8.0"

// Build date and time
#define BUILD_DATE __DATE__
#define BUILD_TIME __TIME__

// OS Information
#define OS_NAME "LitagoDOS"
#define OS_ARCH "x86"
#define OS_PLATFORM "PC"

// Version information structure
struct version_info {
    int major;
    int minor;
    int patch;
    const char* version_string;
    const char* build_date;
    const char* build_time;
    const char* os_name;
    const char* os_arch;
    const char* os_platform;
};

// Function to get version information
const struct version_info* get_version_info(void);

#endif // VERSION_H 