# Visual Debug - Standalone Vulkan test application
# Enable with: -DBUILD_STANDALONE_GLFW_RENDERER=ON

find_package(glfw3 QUIET)

if(NOT glfw3_FOUND)
    message(WARNING "GLFW not found - VkVisualMain target will not be available. "
                    "Install with: sudo apt install libglfw3-dev")
    return()
endif()

set(TARGET_NAME VkVisualMain)

set(SOURCES
        main.cpp
        visual_app.cpp
)

set(HEADERS
        visual_app.h
        glfw_initializer.h
)

collect_sources_recursive(${CMAKE_CURRENT_SOURCE_DIR}/presentation SOURCES HEADERS)

add_executable(${TARGET_NAME}
        ${SOURCES}
        ${HEADERS}
)

target_link_libraries(${TARGET_NAME} PRIVATE
        glfw
        renderer
        logging
        utils
)

target_include_directories(${TARGET_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

message(STATUS "VkVisualMain standalone app target enabled")

