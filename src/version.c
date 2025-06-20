#include "../include/version.h"

static const struct version_info version = {
    .major = VERSION_MAJOR,
    .minor = VERSION_MINOR,
    .patch = VERSION_PATCH,
    .version_string = VERSION_STRING,
    .build_date = BUILD_DATE,
    .build_time = BUILD_TIME,
    .os_name = OS_NAME,
    .os_arch = OS_ARCH,
    .os_platform = OS_PLATFORM
};

const struct version_info* get_version_info(void) {
    return &version;
} 