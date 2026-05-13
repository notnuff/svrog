set(TARGET_NAME editor)

set(LIB_TARGET_NAME editor_lib)

set(LIB_SOURCES editor_app.cpp qml_platform_config.cpp input/qt_input_translator.cpp)
set(LIB_HEADERS editor_app.h qml_initializer.h qml_platform_config.h input/qt_input_translator.h)

add_library(${LIB_TARGET_NAME}
        ${LIB_SOURCES}
        ${LIB_HEADERS}
)

target_link_libraries(${LIB_TARGET_NAME} PUBLIC
        engine
)

target_link_libraries(${LIB_TARGET_NAME} PRIVATE
        Qt6::Core
        Qt6::Quick
        Vulkan::Vulkan
)

target_include_directories(${LIB_TARGET_NAME}
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

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
        editor_lib
        engine
        MainWindowLibplugin
        VulkanViewportLibplugin
)

file(GLOB EDITOR_SHADER_FILES
        ${CMAKE_SOURCE_DIR}/src/engine/shaders/*.slang)
if(EDITOR_SHADER_FILES)
    add_slang_shader_target(editor_shaders
        SOURCES ${EDITOR_SHADER_FILES}
        OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders
    )
    add_dependencies(${TARGET_NAME} editor_shaders)
endif()

file(GLOB EDITOR_TEXTURE_FILES
        ${CMAKE_SOURCE_DIR}/src/engine/textures/*.jpeg
        ${CMAKE_SOURCE_DIR}/src/engine/textures/*.png
        ${CMAKE_SOURCE_DIR}/src/engine/textures/*.jpg)
if(EDITOR_TEXTURE_FILES)
    file(COPY ${EDITOR_TEXTURE_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/textures)
endif()

if(EXISTS ${CMAKE_SOURCE_DIR}/mv_spartan)
    file(COPY ${CMAKE_SOURCE_DIR}/mv_spartan
         DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/models)
endif()
