find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Qml)

qt_standard_project_setup(REQUIRES 6.5)

option(BUILD_EDITOR  "Build Qt/QML editor application"             ON)
option(BUILD_RUNTIME "Build standalone GLFW-based runtime application" ON)

add_subdirectory(logging)
add_subdirectory(utils)
add_subdirectory(engine)

if(BUILD_EDITOR)
    add_subdirectory(editor)
endif()

if(BUILD_RUNTIME)
    add_subdirectory(runtime)
endif()
