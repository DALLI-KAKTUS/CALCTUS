// CALCTUS MicroPython embed configuration. Identical on simulator and ESP32-S3.
#pragma once

#include <port/mpconfigport_common.h>

#define MICROPY_CONFIG_ROM_LEVEL (MICROPY_CONFIG_ROM_LEVEL_CORE_FEATURES)
#define MICROPY_ENABLE_COMPILER (1)
#define MICROPY_ENABLE_GC (1)
#define MICROPY_PY_GC (1)
#define MICROPY_GCREGS_SETJMP (1)
#define MICROPY_NLR_SETJMP (1)
#define MICROPY_STACK_CHECK (1)
#define MICROPY_ENABLE_EXTERNAL_IMPORT (1) // Yalnız microSD kökündeki .py dosyaları.
#define MICROPY_PY_IO (1) // io modülü; open() kök microSD dosyalarına bağlıdır.
#define MICROPY_FLOAT_IMPL (MICROPY_FLOAT_IMPL_DOUBLE)
#define MICROPY_VM_HOOK_LOOP calctus_python_vm_hook();
void calctus_python_vm_hook(void);
#define MICROPY_HW_BOARD_NAME "CALCTUS"
#define MICROPY_HW_MCU_NAME "ESP32-S3 compatible"
