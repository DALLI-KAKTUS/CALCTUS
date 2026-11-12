cmake_minimum_required(VERSION 3.20)
project(calctus LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

# --- Ortak kod ---------------------------------------------------------------
# Simülatörde ve ESP32-S3'te aynı kaynaklar derlenir. Buraya yalnız cihazda da
# çalışan şeyler girer: SDL, işletim sistemi başlıkları, istisna ve RTTI yok.
# tools/check_portable.py bunu her testte denetler.
add_library(calctus_core STATIC
  kernel/kernel.cpp
  kernel/keys.cpp
  kernel/boot.cpp
  ui/canvas.cpp
  ui/font.cpp
  ui/fonts/font_sans12.cpp
  ui/fonts/font_sans16.cpp
  ui/fonts/font_sans24.cpp
  apps/keytest/keytest.cpp
)
target_include_directories(calctus_core PUBLIC hal/include ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(calctus_core PRIVATE
  -fno-exceptions -fno-rtti -Wall -Wextra -Wshadow -Wpedantic)

# --- Masaüstü simülatörü -------------------------------------------------------
option(CALCTUS_BUILD_SIM "Masaüstü simülatörünü derle" ON)
if(CALCTUS_BUILD_SIM)
  find_package(SDL2 REQUIRED)
  add_executable(calctus-sim
    platform/sim/main.cpp
    platform/sim/sim.cpp
    platform/sim/png.cpp
  )
  target_link_libraries(calctus-sim PRIVATE calctus_core SDL2::SDL2)
  target_compile_options(calctus-sim PRIVATE -Wall -Wextra)

  # --- Testler -----------------------------------------------------------------
  enable_testing()
  find_package(Python3 REQUIRED COMPONENTS Interpreter)

  add_test(NAME portable
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/check_portable.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

  add_test(NAME keytest_golden
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/run_golden.py
            $<TARGET_FILE:calctus-sim>
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/scripts/keytest_all.keys
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/golden
            ${CMAKE_CURRENT_BINARY_DIR}/golden-out
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()
