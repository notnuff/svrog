set(TARGET_NAME ${CMAKE_PROJECT_NAME}_app)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Qml)

qt_standard_project_setup(REQUIRES 6.5)

option(BUILD_STANDALONE_GLFW_RENDERER "Build standalone GLFW-based Vulkan renderer" OFF)

add_subdirectory(logging)
add_subdirectory(utils)
add_subdirectory(rendering)

file(GLOB TEXTURE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/rendering/textures/*.jpeg
                        ${CMAKE_CURRENT_SOURCE_DIR}/rendering/textures/*.png
                        ${CMAKE_CURRENT_SOURCE_DIR}/rendering/textures/*.jpg)

if(BUILD_STANDALONE_GLFW_RENDERER)
    add_subdirectory(glfw_debug)

    file(GLOB GLFW_SHADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/rendering/shaders/*.slang)
    if(GLFW_SHADER_FILES)
        add_slang_shader_target(glfw_debug_shaders
            SOURCES ${GLFW_SHADER_FILES}
            OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/glfw_debug/shaders
        )
        add_dependencies(VkVisualMain glfw_debug_shaders)
    endif()

    if(TEXTURE_FILES)
        file(COPY ${TEXTURE_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/glfw_debug/textures)
    endif()
else()
    add_subdirectory(bridge)
    add_subdirectory(ui/VulkanViewport)
    add_subdirectory(ui/MainWindow)

    qt_add_executable(${TARGET_NAME}
            main.cpp
    )

    target_link_libraries(${TARGET_NAME} PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Quick
        Vulkan::Vulkan
        logging
        bridge
        renderer
        MainWindowLibplugin
        VulkanViewportLibplugin
    )

    file(GLOB QML_SHADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/rendering/shaders/*.slang)
    if(QML_SHADER_FILES)
        add_slang_shader_target(qml_shaders
            SOURCES ${QML_SHADER_FILES}
            OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders
        )
        add_dependencies(${TARGET_NAME} qml_shaders)
    endif()

    if(TEXTURE_FILES)
        file(COPY ${TEXTURE_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/textures)
    endif()
endif()