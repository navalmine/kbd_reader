cmake_minimum_required(VERSION 3.16)
find_program(CLANG_C_COMPILER clang)
find_program(CLANG_CXX_COMPILER clang++)
if(NOT CLANG_C_COMPILER OR NOT CLANG_CXX_COMPILER)
  message(FATAL_ERROR "Clang/Clang++ not found. Install clang to build this project.")
endif()
set(CMAKE_C_COMPILER "${CLANG_C_COMPILER}" CACHE STRING "" FORCE)
set(CMAKE_CXX_COMPILER "${CLANG_CXX_COMPILER}" CACHE STRING "" FORCE)

project(codex_demo_kbd_sim LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_QT_APP "Build Qt frontend" ON)

add_subdirectory(lib)
enable_testing()
add_subdirectory(tests)

if(BUILD_QT_APP)
  add_subdirectory(app)
endif()
