
set(TARGET_NAME ${CMAKE_PROJECT_NAME}_app)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Qml)

qt_standard_project_setup(REQUIRES 6.5)

option(BUILD_STANDALONE_GLFW_RENDERER "Build standalone GLFW-based Vulkan renderer" OFF)

add_subdirectory(logging)
add_subdirectory(renderer)

if(BUILD_STANDALONE_GLFW_RENDERER)
    add_subdirectory(glfw_debug)
else()
    add_subdirectory(bridge)
    add_subdirectory(ui/VulkanViewport)
    add_subdirectory(ui/MainWindow)
endif()

qt_add_executable(${TARGET_NAME}
        main.cpp
)

target_link_libraries(${TARGET_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    logging
    bridge
    renderer
    MainWindowLibplugin
    VulkanViewportLibplugin
)