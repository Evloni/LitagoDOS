#ifndef ANSI_H
#define ANSI_H

#include <stdbool.h>

// ANSI escape sequence support
void ansi_set_enabled(bool enabled);
bool ansi_is_enabled(void);

#endif // ANSI_H 