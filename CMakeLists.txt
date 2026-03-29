cmake_minimum_required(VERSION 4.1)

project(svrog LANGUAGES CXX)

find_program(CLANG_TIDY_EXE NAMES clang-tidy)

if(CLANG_TIDY_EXE)
    message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()

find_program(SLANGC_EXECUTABLE NAMES slangc)

if(SLANGC_EXECUTABLE)
    message(STATUS "slangc found: ${SLANGC_EXECUTABLE}")
else()
    message(WARNING "slangc not found - shader compilation will not be available")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src/cmake")
include(SlangShaders)

add_subdirectory(src)