
set(TARGET_NAME ${CMAKE_PROJECT_NAME}_app)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Qml)

qt_standard_project_setup(REQUIRES 6.5)

add_subdirectory(ui/VulkanViewport)
add_subdirectory(ui/MainWindow)
add_subdirectory(renderer)
add_subdirectory(bridge)

qt_add_executable(${TARGET_NAME}
        main.cpp
)

target_link_libraries(${TARGET_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    bridge
    renderer
    MainWindowLibplugin
    VulkanViewportLibplugin
)