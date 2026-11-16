cmake_minimum_required(VERSION 3.20)
project(calctus LANGUAGES C CXX)

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
include(cmake/core_sources.cmake)
set(CALCTUS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
include(cmake/micropython_sources.cmake)
file(GLOB_RECURSE CALCTUS_LVGL_SOURCES CONFIGURE_DEPENDS third_party/lvgl/src/*.c)
add_library(calctus_lvgl STATIC ${CALCTUS_LVGL_SOURCES})
target_include_directories(calctus_lvgl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
add_library(calctus_micropython STATIC ${CALCTUS_MICROPYTHON_SOURCES})
target_include_directories(calctus_micropython PUBLIC
  services/micropython ${CALCTUS_MICROPYTHON_ROOT})
target_compile_definitions(calctus_micropython PRIVATE NDEBUG)
add_library(calctus_core STATIC ${CALCTUS_CORE_SOURCES})
target_include_directories(calctus_core PUBLIC hal/include ${CMAKE_CURRENT_SOURCE_DIR})
add_library(calctus_libbf STATIC ${CALCTUS_LIBBF_SOURCES})
target_include_directories(calctus_libbf PRIVATE third_party/libbf)
target_include_directories(calctus_core PRIVATE third_party/libbf)
target_link_libraries(calctus_core PUBLIC calctus_libbf calctus_micropython calctus_lvgl)
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
  add_executable(calctus-math-test tests/math_test.cpp)
  target_link_libraries(calctus-math-test PRIVATE calctus_core)
  add_test(NAME math COMMAND calctus-math-test)
  add_executable(calctus-statistics-test tests/statistics_test.cpp)
  target_link_libraries(calctus-statistics-test PRIVATE calctus_core)
  add_test(NAME statistics COMMAND calctus-statistics-test)
  add_executable(calctus-combinatorics-test tests/combinatorics_test.cpp)
  target_link_libraries(calctus-combinatorics-test PRIVATE calctus_core)
  add_test(NAME combinatorics COMMAND calctus-combinatorics-test)
  add_executable(calctus-solver-test tests/solver_test.cpp)
  target_link_libraries(calctus-solver-test PRIVATE calctus_core)
  add_test(NAME solver COMMAND calctus-solver-test)
  add_executable(calctus-sequence-test tests/sequence_test.cpp)
  target_link_libraries(calctus-sequence-test PRIVATE calctus_core)
  add_test(NAME sequence COMMAND calctus-sequence-test)
  add_executable(calctus-elements-test tests/elements_test.cpp)
  target_link_libraries(calctus-elements-test PRIVATE calctus_core)
  add_test(NAME elements COMMAND calctus-elements-test)
  add_executable(calctus-tca8418-test tests/tca8418_test.cpp)
  target_include_directories(calctus-tca8418-test PRIVATE hal/include ${CMAKE_CURRENT_SOURCE_DIR})
  add_test(NAME tca8418 COMMAND calctus-tca8418-test)
  add_executable(calctus-python-runtime-test tests/python_runtime_test.cpp)
  target_link_libraries(calctus-python-runtime-test PRIVATE calctus_core)
  add_test(NAME python_runtime COMMAND calctus-python-runtime-test)
  add_executable(calctus-pi-link-test tests/pi_link_test.cpp services/pi_link.cpp)
  target_include_directories(calctus-pi-link-test PRIVATE hal/include ${CMAKE_CURRENT_SOURCE_DIR})
  add_test(NAME pi_link COMMAND calctus-pi-link-test)
  find_package(Python3 REQUIRED COMPONENTS Interpreter)
  add_test(NAME pi_agent COMMAND ${Python3_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/pi_agent_test.py)
  add_test(NAME pi_app COMMAND ${Python3_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/pi_app_test.py $<TARGET_FILE:calctus-sim>
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/golden/pi_app)

  add_test(NAME portable
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/check_portable.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

  add_test(NAME storage
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/storage_test.py
            $<TARGET_FILE:calctus-sim>)
  add_test(NAME files
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/files_test.py
            $<TARGET_FILE:calctus-sim>)
  add_test(NAME python_app
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/python_app_test.py
            $<TARGET_FILE:calctus-sim> ${CMAKE_CURRENT_SOURCE_DIR}/tests/golden/python_files)
  add_test(NAME python_keypad
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/python_keypad_test.py
            $<TARGET_FILE:calctus-sim> ${CMAKE_CURRENT_SOURCE_DIR}/tests/scripts/python_keypad.keys
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/golden/python_keypad/03_sonuc.png)
  add_test(NAME python_example
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/python_example_test.py
            $<TARGET_FILE:calctus-sim> ${CMAKE_CURRENT_SOURCE_DIR}/examples/python/calctus_demo.py
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/golden/python_example.png)

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
