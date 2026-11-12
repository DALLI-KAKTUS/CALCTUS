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
  kernel/system.cpp
  kernel/keys.cpp
  kernel/boot.cpp
  ui/canvas.cpp
  ui/font.cpp
  ui/theme.cpp
  ui/fonts/font_sans12.cpp
  ui/fonts/font_sans16.cpp
  ui/fonts/font_sans24.cpp
  ui/widgets/list.cpp
  ui/widgets/table.cpp
  ui/widgets/modal.cpp
  apps/home/home.cpp
  apps/settings/settings_app.cpp
  apps/gallery/gallery.cpp
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

  # tests/scripts altındaki her tuş betiği bir ekran karşılaştırma testidir.
  file(GLOB CALCTUS_SCRIPTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/scripts/*.keys)
  foreach(script ${CALCTUS_SCRIPTS})
    get_filename_component(name ${script} NAME_WE)
    add_test(NAME golden_${name}
      COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/run_golden.py
              $<TARGET_FILE:calctus-sim> ${script}
              ${CMAKE_CURRENT_SOURCE_DIR}/tests/golden/${name}
              ${CMAKE_CURRENT_BINARY_DIR}/golden-out/${name}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  endforeach()
endif()
