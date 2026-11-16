# Generated and pinned MicroPython embed package, see third_party/micropython_embed/README.md.
set(CALCTUS_MICROPYTHON_ROOT "${CALCTUS_ROOT}/third_party/micropython_embed")
file(GLOB_RECURSE CALCTUS_MICROPYTHON_SOURCES
     "${CALCTUS_MICROPYTHON_ROOT}/*.c")
# Replace upstream stdout printer with a bounded UI output hook.
list(REMOVE_ITEM CALCTUS_MICROPYTHON_SOURCES "${CALCTUS_MICROPYTHON_ROOT}/port/mphalport.c")
list(APPEND CALCTUS_MICROPYTHON_SOURCES "${CALCTUS_ROOT}/services/micropython/port.c")
