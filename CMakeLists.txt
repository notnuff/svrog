cmake_minimum_required(VERSION 4.1)

project(svrog LANGUAGES CXX)

find_program(CLANG_TIDY_EXE NAMES clang-tidy)

if(CLANG_TIDY_EXE)
    message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()

if(DEFINED ENV{VULKAN_SDK})
    find_program(SLANGC_EXECUTABLE
        NAMES slangc
        PATHS "$ENV{VULKAN_SDK}/bin"
        NO_DEFAULT_PATH
    )
endif()

if(NOT SLANGC_EXECUTABLE)
    find_program(SLANGC_EXECUTABLE NAMES slangc)
endif()

if(SLANGC_EXECUTABLE)
    message(STATUS "slangc found: ${SLANGC_EXECUTABLE}")
else()
    message(WARNING "slangc not found - shader compilation will not be available. "
                    "Make sure VULKAN_SDK environment variable is set.")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src/cmake")
include(SlangShaders)

add_subdirectory(src)