# Visual Debug - Standalone Vulkan test application
# Enable with: cmake -DENABLE_STANDALONE_VK_APP_BUILD=ON ..

find_package(Vulkan REQUIRED)
find_package(glfw3 QUIET)

if(NOT glfw3_FOUND)
    message(WARNING "GLFW not found - VkVisualMain target will not be available. "
                    "Install with: sudo apt install libglfw3-dev")
    return()
endif()

set(TARGET_NAME VkVisualMain)

set(SOURCES
        main.cpp
        vk_visual_app.cpp
)

set(HEADERS
        vk_visual_app.h
)

add_executable(${TARGET_NAME}
        ${SOURCES}
        ${HEADERS}
)

target_link_libraries(${TARGET_NAME} PRIVATE
        Vulkan::Vulkan
        glfw
        renderer
)

target_include_directories(${TARGET_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_compile_definitions(${TARGET_NAME} PRIVATE
        GLFW_INCLUDE_VULKAN
)

message(STATUS "VkVisualMain standalone app target enabled")

