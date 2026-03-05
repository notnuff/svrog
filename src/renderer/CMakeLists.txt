set(TARGET_NAME renderer)

set(SOURCES
        vk_instance.cpp
        vk_ctx.cpp
        vk_builder.cpp
        vk_initializer.cpp
)

set(HEADERS
        vk_instance.h
        vk_ctx.h
        vk_builder.h
        vk_initializer.h
        shaders/triangle_shaders.h
)

include_directories(. PUBLIC)
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

# standalone Vulkan debug window
find_package(Vulkan REQUIRED)
find_package(glfw3 QUIET)

if(glfw3_FOUND)
    add_executable(VkVisualMain
            vk_visual_main.cpp
    )

    target_link_libraries(VkVisualMain PRIVATE
            Vulkan::Vulkan
            glfw
            ${TARGET_NAME}
    )

    target_compile_definitions(VkVisualMain PRIVATE
            GLFW_INCLUDE_VULKAN
    )

    message(STATUS "VkVisualMain target enabled (GLFW found)")
else()
    message(STATUS "VkVisualMain target disabled (GLFW not found - install with: sudo apt install libglfw3-dev)")
endif()