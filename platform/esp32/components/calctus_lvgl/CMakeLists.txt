get_filename_component(CALCTUS_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../.." ABSOLUTE)
file(GLOB_RECURSE CALCTUS_LVGL_SOURCES "${CALCTUS_ROOT}/third_party/lvgl/src/*.c")
idf_component_register(SRCS ${CALCTUS_LVGL_SOURCES} INCLUDE_DIRS "${CALCTUS_ROOT}")
