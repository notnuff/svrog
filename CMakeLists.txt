# TODO: refactor, separate QML Qt's entry point and the engine itself, figure out what every command does?

cmake_minimum_required(VERSION 3.16)
project(svrog LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick)

qt_standard_project_setup()

qt_add_executable(svrog WIN32 MACOSX_BUNDLE
        main.cpp
        vulkandemorenderer.cpp vulkandemorenderer.h
        vulkanviewport.cpp vulkanviewport.h
        vulkanrenderer.h
)

set_target_properties(svrog PROPERTIES
    # Prevent name clash with build subdirectory on case-insensitive file systems
    OUTPUT_NAME svrogapp
)

target_link_libraries(svrog PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
)

qt_add_qml_module(svrog
    URI VulkanViewport
    QML_FILES
        main.qml
    RESOURCES
        demo.frag.spv
        demo.vert.spv
    RESOURCE_PREFIX /scenegraph/demo
    NO_RESOURCE_TARGET_PATH
)

install(TARGETS svrog
    BUNDLE  DESTINATION svrog
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET svrog
    OUTPUT_SCRIPT deploy_script
    MACOS_BUNDLE_POST_BUILD
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)
install(SCRIPT ${deploy_script})
