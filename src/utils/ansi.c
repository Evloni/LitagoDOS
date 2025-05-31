#include "../../include/ansi.h"

static bool ansi_enabled = false;

void ansi_set_enabled(bool enabled) {
    ansi_enabled = enabled;
}

bool ansi_is_enabled(void) {
    return ansi_enabled;
} 