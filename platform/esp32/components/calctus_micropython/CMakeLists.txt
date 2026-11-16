get_filename_component(CALCTUS_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../.." ABSOLUTE)
include("${CALCTUS_ROOT}/cmake/micropython_sources.cmake")
idf_component_register(SRCS ${CALCTUS_MICROPYTHON_SOURCES}
  INCLUDE_DIRS "${CALCTUS_ROOT}/services/micropython" "${CALCTUS_MICROPYTHON_ROOT}")
target_compile_definitions(${COMPONENT_LIB} PRIVATE NDEBUG)
