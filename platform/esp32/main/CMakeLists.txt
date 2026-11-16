get_filename_component(CALCTUS_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
include("${CALCTUS_ROOT}/cmake/core_sources.cmake")
set(CALCTUS_IDF_SOURCES "hal.cpp" "main.cpp")
foreach(source IN LISTS CALCTUS_CORE_SOURCES CALCTUS_LIBBF_SOURCES)
  list(APPEND CALCTUS_IDF_SOURCES "${CALCTUS_ROOT}/${source}")
endforeach()
idf_component_register(
  SRCS ${CALCTUS_IDF_SOURCES}
  INCLUDE_DIRS "${CALCTUS_ROOT}" "${CALCTUS_ROOT}/hal/include"
               "${CALCTUS_ROOT}/third_party/libbf"
  REQUIRES esp_lcd esp_driver_spi esp_driver_i2c esp_driver_gpio esp_driver_ledc esp_driver_uart
            esp_timer nvs_flash esp_psram fatfs sdmmc calctus_micropython calctus_lvgl)
target_compile_options(${COMPONENT_LIB} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
                                                $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>)
