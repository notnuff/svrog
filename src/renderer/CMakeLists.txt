cmake_minimum_required(VERSION 4.1)

project(renderer LANGUAGES CXX)

set(TARGET_NAME renderer)

find_package(Vulkan REQUIRED)

option(BUILD_STANDALONE_GLFW_RENDERER "Build standalone GLFW-based Vulkan renderer" OFF)

function(collect_sources_recursive DIR SOURCES_VAR HEADERS_VAR)
    file(GLOB LOCAL_SOURCES "${DIR}/*.cpp")
    file(GLOB LOCAL_HEADERS "${DIR}/*.h")

    set(${SOURCES_VAR} ${${SOURCES_VAR}} ${LOCAL_SOURCES} PARENT_SCOPE)
    set(${HEADERS_VAR} ${${HEADERS_VAR}} ${LOCAL_HEADERS} PARENT_SCOPE)

    file(GLOB SUBDIRS RELATIVE ${DIR} ${DIR}/*)

    foreach(SUBDIR ${SUBDIRS})
        set(FULL_SUBDIR "${DIR}/${SUBDIR}")

        if(IS_DIRECTORY ${FULL_SUBDIR})
            if(NOT EXISTS "${FULL_SUBDIR}/CMakeLists.txt")
                collect_sources_recursive(${FULL_SUBDIR} ${SOURCES_VAR} ${HEADERS_VAR})
                set(${SOURCES_VAR} ${${SOURCES_VAR}} PARENT_SCOPE)
                set(${HEADERS_VAR} ${${HEADERS_VAR}} PARENT_SCOPE)
            endif()
        endif()
    endforeach()
endfunction()

set(SOURCES "")
set(HEADERS "")
collect_sources_recursive(${CMAKE_CURRENT_SOURCE_DIR} SOURCES HEADERS)

add_library(${TARGET_NAME}
        ${SOURCES}
        ${HEADERS}
)

target_link_libraries(${TARGET_NAME} PRIVATE
        Vulkan::Vulkan
)

target_include_directories(${TARGET_NAME}
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

if(BUILD_STANDALONE_GLFW_RENDERER)
    add_subdirectory(visual_debug)
endif()