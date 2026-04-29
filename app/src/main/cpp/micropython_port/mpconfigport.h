/* MicroPython configuration for the Android NDK embedding spike. */

#pragma once

#include <port/mpconfigport_common.h>

#define MICROPY_CONFIG_ROM_LEVEL (MICROPY_CONFIG_ROM_LEVEL_MINIMUM)
#define MICROPY_ENABLE_COMPILER (1)
#define MICROPY_ENABLE_GC (1)
#define MICROPY_PY_GC (1)
#define MICROPY_NLR_SETJMP (1)
#define MICROPY_USE_INTERNAL_PRINTF (0)
