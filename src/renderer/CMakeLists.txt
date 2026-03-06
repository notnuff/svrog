cmake_minimum_required(VERSION 4.1)

project(renderer LANGUAGES CXX)

set(TARGET_NAME renderer)

find_package(Vulkan REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core)

option(BUILD_STANDALONE_GLFW_RENDERER "Build standalone GLFW-based Vulkan renderer" OFF)

function(collect_sources_recursive DIR SOURCES_VAR HEADERS_VAR)
    file(GLOB LOCAL_SOURCES "${DIR}/*.cpp")
    file(GLOB LOCAL_HEADERS "${DIR}/*.h")

    list(APPEND ${SOURCES_VAR} ${LOCAL_SOURCES})
    list(APPEND ${HEADERS_VAR} ${LOCAL_HEADERS})

    file(GLOB SUBDIRS RELATIVE ${DIR} ${DIR}/*)

    foreach(SUBDIR ${SUBDIRS})
        set(FULL_SUBDIR "${DIR}/${SUBDIR}")

        if(IS_DIRECTORY ${FULL_SUBDIR})
            # Skip directories with CMakeLists.txt,
            # as their sources must be handled by respective CMakeLists.txt
            if(NOT EXISTS "${FULL_SUBDIR}/CMakeLists.txt")
                collect_sources_recursive(${FULL_SUBDIR} ${SOURCES_VAR} ${HEADERS_VAR})
            endif()
        endif()
    endforeach()

    set(${SOURCES_VAR} ${${SOURCES_VAR}} PARENT_SCOPE)
    set(${HEADERS_VAR} ${${HEADERS_VAR}} PARENT_SCOPE)
endfunction()

set(SOURCES "")
set(HEADERS "")
collect_sources_recursive(${CMAKE_CURRENT_SOURCE_DIR}/vk_ctx SOURCES HEADERS)
collect_sources_recursive(${CMAKE_CURRENT_SOURCE_DIR}/shaders SOURCES HEADERS)

add_library(${TARGET_NAME}
        ${SOURCES}
        ${HEADERS}
)

target_link_libraries(${TARGET_NAME} PUBLIC
        Vulkan::Vulkan
        Qt6::Core
)

target_include_directories(${TARGET_NAME}
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

if(BUILD_STANDALONE_GLFW_RENDERER)
    add_subdirectory(visual_debug)
endif()